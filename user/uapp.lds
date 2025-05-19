OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS
{
    . = 0x1000;

    .text : {
        _stext_user = .;
        *(.text.init)
        *(.text .text.*)
        _etext_user = .;
    }

    .rodata : {
        _srodata_user = .;
        *(.rodata .rodata*)
        _erodata_user = .;
    }

    .data : {
        _sdata_user = .;
        *(.data .data.*)

        /*
         * Note for next year TAs, from 45gfg9:
         *
         * Currently we can't use a dedicated section for .bss. They don't
         * take up space in the stripped binary, so when uapp is mapped,
         * attempts to access them will cause a page fault.
         *
         * Adding .bss to .data section will force them to take up space
         * in the binary. This is a workaround for the issue.
         *
         * If you know what I'm talking about, then introduce ELF parser
         * to the kernel and fix this issue, just like OS courses do.
         * See https://zju-sec.github.io/os24fall-stu/lab4/#elf_1
         */
        *(.sbss .sbss.*)
        *(.bss .bss.*)
        _edata_user = .;
    }
}
