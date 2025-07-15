C_SOURCES = $(wildcard kernel/*.c drivers/*.c klib/*.c arch/*.c)
S_SOURCES = $(wildcard boot/*.s)
OBJ = $(C_SOURCES:.c=.o)

CC = ~/opt/cross-riscv/bin/riscv64-elf-gcc
AS = ~/opt/cross-riscv/bin/riscv64-elf-as
LN = ~/opt/cross-riscv/bin/riscv64-elf-ld
GDB= ~/opt/cross-riscv/bin/riscv64-elf-gdb
OBJCPY= ~/opt/cross-riscv/bin/riscv64-elf-objcopy

CFLAGS = -Wall -Wextra -mcmodel=medany -ffreestanding -g
LFLAGS = -T linker.ld -nostdlib -o kernel.elf

QEMUOPTS = -machine virt -cpu rv64,sv39=on -bios default 
QEMUOPTS += -device virtio-gpu-pci -m 1G # 1 Gibibytes of RAM
QEMUOPTS += -monitor mon:stdio
#QEMUOPTS += -d unimp,guest_errors,int,cpu_reset -D qemu.log # Specific to debugging traps, comment when not in use. (MAKES LARGE FILES!)


generate_dts: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -machine dumpdtb=riscv64-virt.dtb
	@dtc -I dtb -O dts riscv64-virt.dtb -o riscv64-virt.dts
	@rm riscv64-virt.dtb
	@mv riscv64-virt.dts misc/riscv64-virt.dts

debug: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -s -S &
	$(GDB) kernel.elf 

run: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf 

build: kernel.elf
	@echo "Built!"

kernel.elf: ${OBJ} boot/entry.o
	@$(LN) $^ $(LFLAGS)
	$(info ${OBJ})

################################### Wildcard Rules ###################################
%.o: %.c 
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	@$(AS) $^ -o $@


################################### Phony Targets ####################################
.PHONY: clean
clean:
	@find . -type f -name '*.o' -delete
	@find . -type f -name 'kernel.sym' -delete
	@find . -type f -name '*.elf' -delete
	@find . -type f -name 'kernel.bin' -delete
	