#ifndef __VM_H__
#define __VM_H__

#include <private_kdefs.h>
#include <stdint.h>

#define SV39_PTE_V 0x1
#define SV39_PTE_R 0x2
#define SV39_PTE_W 0x4
#define SV39_PTE_X 0x8
#define SV39_PTE_U 0x10
#define SV39_PTE_G 0x20
#define SV39_PTE_A 0x40
#define SV39_PTE_D 0x80
#define SV39_EARLY_PTE_PPN_BEGIN 28
#define SV39_PTE_PPN_BEGIN 10
#define SV39_PTE_PPN_END 53
#define SV39_PTE_PPN_LENGTH (SV39_PTE_PPN_END - SV39_PTE_PPN_BEGIN + 1)
#define SV39_VPN_LEN 9

#define GET_SUBBITMAP(bitmap, low, high) ((bitmap) >> (low) & ((1ULL << ((high) - (low) + 1)) - 1))
#define TRUNCATE(bitmap, num_bits) ((bitmap) & ((1ULL << (num_bits)) - 1))

#define PPN2PHYS(x) ((uint64_t)(x) << 12)
#define PHYS2PPN(x) ((uint64_t)(x) >> 12)

/**
 * @brief 设置内核初始化阶段的页表映射关系
 */
void setup_vm(void);

/**
 * @brief 设置内核最终的页表映射关系
 */
void setup_vm_final(void);

/**
 * @brief 创建多级页表映射关系
 *
 * 在指定的一段虚拟内存 va 创建映射关系，将其映射到物理内存 pa
 *
 * @param pgtbl 根页表的基地址
 * @param va 虚拟地址
 * @param pa 物理地址
 * @param sz 映射的大小
 * @param perm 映射的读写权限
 */
void create_mapping(uint64_t pgtbl[static PGSIZE / 8], void *va, void *pa, uint64_t sz, uint64_t perm);

uint64_t* get_physical_address(uint64_t pgtbl[static PGSIZE / 8], uint64_t* va);

#endif
