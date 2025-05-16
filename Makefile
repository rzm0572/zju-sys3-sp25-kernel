CROSS_ := riscv64-unknown-linux-gnu-
export GCC := ${CROSS_}gcc
export LD := ${CROSS_}ld
export OBJCOPY := ${CROSS_}objcopy
export OBJDUMP := ${CROSS_}objdump
export NM := ${CROSS_}nm

ISA=rv64i_zicsr
ABI=lp64

export INCLUDE := -I $(shell pwd)/include -I $(shell pwd)/arch/riscv/include
export CF := -march=$(ISA) -mabi=$(ABI) -mcmodel=medany \
	-fno-builtin -ffunction-sections -fdata-sections \
	-nostartfiles -nostdlib -nostdinc -static -ggdb -Og \
	-Wall -Wextra -std=gnu11 \
	-lgcc -Wl,--nmagic -Wl,--gc-sections -g 
export DSJF := -DSJF
export DPRIORITY := -DPRIORITY
export CFLAG := ${CF} ${INCLUDE} ${DSJF}
# CFLAG = ${CF} ${INCLUDE} ${DPRIORITY}

.PHONY:all run debug clean
all:
	${MAKE} -C lib all
#	${MAKE} -C init all
	${MAKE} -C arch/riscv all
	$(LD) -T arch/riscv/kernel/vmlinux.lds arch/riscv/kernel/*.o lib/*.o -o vmlinux
	mkdir -p arch/riscv/boot
	$(OBJCOPY) -O binary vmlinux arch/riscv/boot/Image
	$(OBJDUMP) -S vmlinux > vmlinux.asm
	$(NM) -n vmlinux > System.map
	@echo -e '\n'Build Finished OK

run: all
	@echo Launch the qemu ......
	@qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -bios default 

debug: all
	@echo Launch the qemu for debug ......
	@qemu-system-riscv64 -nographic -machine virt -kernel vmlinux -bios default -S -s

SPIKE_CONF = $(CURDIR)/../../../repo/sys-3-project/spike/
spike_run:all
	spike --kernel=arch/riscv/boot/Image $(SPIKE_CONF)/fw_jump.elf

spike_debug:all
	spike -H --rbb-port=9824 --kernel=arch/riscv/boot/Image $(SPIKE_CONF)/fw_jump.elf

spike_bridge:
	openocd -f $(SPIKE_CONF)/spike.cfg

clean:
	${MAKE} -C lib clean
	${MAKE} -C arch/riscv clean
	$(shell test -f vmlinux && rm vmlinux)
	$(shell test -f System.map && rm System.map)
	@echo -e '\n'Clean Finished
