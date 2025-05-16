#include <vm.h>
#include <mm.h>
#include <printk.h>
#include <private_kdefs.h>
#include <stdint.h>
#include <string.h>
#include <csr.h>

extern uint8_t _stext[];
extern uint8_t _etext[];
extern uint8_t _srodata[];
extern uint8_t _erodata[];
extern uint8_t _skernel[];
extern uint8_t _ekernel[];
extern uint8_t _sdata[];
extern uint8_t _edata[];
extern uint8_t _sbss[];
extern uint8_t _ebss[];


#define PTE_HAS_PERM(pte, perm) ((pte) & SV39_PTE_##perm)

#define GIGAPAGE_SHIFT 18
#define GIGAPAGE_PPN GET_SUBBITMAP(PHY_START, 30, 38)
#define GIGAPAGE_VPN GET_SUBBITMAP(VM_START, 30, 38)

// 用于 setup_vm 进行 1 GiB 的映射
uint64_t early_pgtbl[PGSIZE / 8] __attribute__((__aligned__(PGSIZE)));
// kernel page table 根目录，在 setup_vm_final 进行映射
uint64_t swapper_pg_dir[PGSIZE / 8] __attribute__((__aligned__(PGSIZE)));

uint64_t extract_bits(uint64_t* val, uint64_t num_bits) {
    uint64_t ret = *val & ((1 << num_bits) - 1);
    *val >>= num_bits;
    return ret;
}

void setup_vm(void) {
    memset(early_pgtbl, 0, PGSIZE);

    // 1. 初始化阶段，页大小为 1 GiB，不使用多级页表
    // 2. 将 va 的 64 bit 作如下划分：| 63...39 | 38...30 | 29...0 |
    //    - 63...39 bit 忽略
    //    - 38...30 bit 作为 early_pgtbl 的索引
    //    - 29...0 bit 作为页内偏移，注意到 30 = 9 + 9 +
    //    12，即我们此处只使用根页表，根页表的每个 entry 对应 1 GiB 的页
    // 3. Page Table Entry 的权限为 X W R V

    uint64_t gigapage_ppn = (uint64_t)GIGAPAGE_PPN;
    uint64_t gigapage_vpn = (uint64_t)GIGAPAGE_VPN;
    uint64_t ppn = gigapage_ppn << GIGAPAGE_SHIFT;

    early_pgtbl[gigapage_ppn] = SV39_PTE(ppn, SV39_PTE_V | SV39_PTE_R | SV39_PTE_W | SV39_PTE_X | SV39_PTE_A | SV39_PTE_D);
    early_pgtbl[gigapage_vpn] = SV39_PTE(ppn, SV39_PTE_V | SV39_PTE_R | SV39_PTE_W | SV39_PTE_X | SV39_PTE_A | SV39_PTE_D);
}

void setup_vm_final(void) {
    memset(swapper_pg_dir, 0, PGSIZE);

    // No OpenSBI mapping required

    // 1. 调用 create_mapping 映射页表
    //    - kernel code: X R
    //    - kernel rodata: R
    //    - other memory: W R

    create_mapping(swapper_pg_dir, _stext, (uint8_t*)VA2PA(_stext), (uint64_t)(_etext - _stext), SV39_PTE_X | SV39_PTE_R | SV39_PTE_A);
    create_mapping(swapper_pg_dir, _srodata, (uint8_t*)VA2PA(_srodata),  (uint64_t)(_erodata - _srodata), SV39_PTE_R | SV39_PTE_A);
    create_mapping(swapper_pg_dir, _sdata, (uint8_t*)VA2PA(_sdata), (uint64_t)_skernel + PHY_SIZE - (uint64_t)_sdata, SV39_PTE_W | SV39_PTE_R | SV39_PTE_A | SV39_PTE_D);

    // 2. 设置 satp，将 swapper_pg_dir 作为内核页表
    uint64_t top_ppn = PHYS2PPN(VA2PA(swapper_pg_dir));
    csr_write(satp, top_ppn | SATP_MODE);

    // flush TLB
    asm volatile("sfence.vma" ::: "memory");

    return;
}


void create_mapping_page(uint64_t pgtbl[static PGSIZE / 8], uint64_t _vpn, uint64_t _ppn, uint64_t perm) {
    uint64_t vpn[3] = {
        extract_bits(&_vpn, SV39_VPN_LEN),
        extract_bits(&_vpn, SV39_VPN_LEN),
        extract_bits(&_vpn, SV39_VPN_LEN)
    };

    uint64_t* page_table = (uint64_t*)pgtbl;
    for (int i = 2; i >= 1; i--) {
        uint64_t pte = page_table[vpn[i]];
        uint64_t ppn = GET_SUBBITMAP(pte, SV39_PTE_PPN_BEGIN, SV39_PTE_PPN_END);
        uint64_t* new_page_table = (uint64_t*)PA2VA(PPN2PHYS(ppn));
        
        if (!PTE_HAS_PERM(pte, V)) {
            new_page_table = (uint64_t*)alloc_page();
            memset(new_page_table, 0, PGSIZE);
            uint64_t new_ppn = PHYS2PPN(VA2PA(new_page_table));
            page_table[vpn[i]] = SV39_PTE(new_ppn, SV39_PTE_V);
        }

        page_table = new_page_table;
    }

    perm |= SV39_PTE_V;
    page_table[vpn[0]] = SV39_PTE(_ppn, perm);
}

void create_mapping(uint64_t pgtbl[static PGSIZE / 8], void *va, void *pa,
                    uint64_t sz, uint64_t perm) {
    // TODO：根据 RISC-V Sv39 的要求，创建多级页表映射关系
    //
    // 物理内存需要分页
    // 创建多级页表的时候使用 alloc_page 来获取新的一页作为页表
    // 注意通过 V bit 来判断表项是否存在
    //
    // 重要：阅读手册，注意 A / D 位的设置

    uint64_t vpn_low = (uint64_t)va >> PAGE_SHIFT;
    uint64_t vpn_high = ((uint64_t)va + sz - 1) >> PAGE_SHIFT;
    uint64_t ppn_low = (uint64_t)pa >> PAGE_SHIFT;

    for (uint64_t vpn = vpn_low, ppn = ppn_low; vpn <= vpn_high; vpn++, ppn++) {
        create_mapping_page(pgtbl, vpn, ppn, perm);
    }

    printk("pgtbl = 0x%" PRIx64 ": map [0x%" PRIx64 ", 0x%" PRIx64 ") -> [0x%" PRIx64 ", 0x%" PRIx64 "), perm = 0x%" PRIx64 ", size=%" PRId64 "\n", (uint64_t)pgtbl, (uint64_t)va, (uint64_t)va + sz, (uint64_t)pa, (uint64_t) pa + sz, perm, sz);
}

void copy_mapping(uint64_t dst_pgtbl[static PGSIZE / 8], uint64_t src_pgtbl[static PGSIZE / 8]) {
    for (uint64_t i = 0; i < PGSIZE / 8; i++) {
        if (src_pgtbl[i] & SV39_PTE_V) {
            uint64_t src_ppn_lv2 = SV39_GET_PPN(src_pgtbl[i]);
            uint64_t *src_pgtbl_lv2 = (uint64_t*)PA2VA(PPN2PHYS(src_ppn_lv2));
            uint64_t *dst_pgtbl_lv2 = (uint64_t*)alloc_page();
            uint64_t dst_ppn_lv2 = PHYS2PPN(VA2PA(dst_pgtbl_lv2));
            dst_pgtbl[i] = SV39_PTE(dst_ppn_lv2, SV39_GET_PERM(src_pgtbl[i]));
            
            for (uint64_t j = 0; j < PGSIZE / 8; j++) {
                if (src_pgtbl_lv2[j] & SV39_PTE_V) {
                    uint64_t src_ppn_lv3 = SV39_GET_PPN(src_pgtbl_lv2[j]);
                    uint64_t *src_pgtbl_lv3 = (uint64_t*)PA2VA(PPN2PHYS(src_ppn_lv3));
                    uint64_t *dst_pgtbl_lv3 = (uint64_t*)alloc_page();
                    uint64_t dst_ppn_lv3 = PHYS2PPN(VA2PA(dst_pgtbl_lv3));
                    dst_pgtbl_lv2[j] = SV39_PTE(dst_ppn_lv3, SV39_GET_PERM(src_pgtbl_lv2[j]));
                    memcpy(dst_pgtbl_lv3, src_pgtbl_lv3, PGSIZE);
                }
            }
        }
    }
}

int is_valid_pte(uint64_t* pte_ptr) {
    if (pte_ptr == INVALID_PA) {
        return 0;
    }
    if (!(*pte_ptr & SV39_PTE_V)) {
        return 0;
    }
    return 1;
}

uint64_t* walk_page_table_pte(uint64_t pgtbl[static PGSIZE / 8], uint64_t _vpn) {
    uint64_t* va_ptr = (uint64_t*)&_vpn;
    uint64_t vpn[3] = {
        extract_bits(va_ptr, SV39_VPN_LEN),
        extract_bits(va_ptr, SV39_VPN_LEN),
        extract_bits(va_ptr, SV39_VPN_LEN)
    };

    uint64_t* page_table = (uint64_t*)pgtbl;
    for (int i = 2; i >= 1; i--) {
        uint64_t pte = page_table[vpn[i]];
        if (!(pte & SV39_PTE_V)) {
            return INVALID_PA;
        }
        uint64_t ppn = GET_SUBBITMAP(pte, SV39_PTE_PPN_BEGIN, SV39_PTE_PPN_END);
        page_table = (uint64_t*)PA2VA(PPN2PHYS(ppn));
    }

    return &page_table[vpn[0]];
}

uint64_t* walk_page_table(uint64_t pgtbl[static PGSIZE / 8], uint64_t* va) {
    uint64_t* va_ptr = (uint64_t*)&va;
    uint64_t offset = extract_bits(va_ptr, PAGE_SHIFT);
    uint64_t* pte_ptr = walk_page_table_pte(pgtbl, (uint64_t)va);
    
    if (!is_valid_pte(pte_ptr)) {
        return INVALID_PA;
    }

    uint64_t pte = *pte_ptr;
    uint64_t ppn = GET_SUBBITMAP(pte, SV39_PTE_PPN_BEGIN, SV39_PTE_PPN_END);
    return (uint64_t*)((ppn << PAGE_SHIFT) + offset);
}
