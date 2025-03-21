C_SOURCES = $(wildcard kernel/*.c drivers/*.c kernel/stdlib/*.c)
OBJ = $(C_SOURCES:.c=.o)

CC = ~/opt/cross-riscv/bin/riscv64-elf-gcc
AS = ~/opt/cross-riscv/bin/riscv64-elf-as
LN = ~/opt/cross-riscv/bin/riscv64-elf-ld

CFLAGS = -Wall -Wextra -mcmodel=medany -ffreestanding
LFLAGS = -T linker.ld -nostdlib -o kernel.elf

QEMUOPTS = -machine virt -bios none
QEMUOPTS += -monitor mon:stdio
QEMUOPTS += -device virtio-gpu-pci -m 4830196K

generate_dts:
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -machine dumpdtb=riscv64-virt.dtb
	@dtc -I dtb -O dts riscv64-virt.dtb -o riscv64-virt.dts
	@rm riscv64-virt.dtb
	@mv riscv64-virt.dts misc/riscv64-virt.dts

run: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf 

run_sifive_u: kernel.elf
	@echo "Running QEMU for SiFive Unleashed board"
	@qemu-system-riscv64 -machine sifive_u -bios none -kernel kernel.elf -serial mon:stdio

build: kernel.elf
	@echo "Built!"

kernel.elf: ${OBJ} boot/entry.o
	@$(LN) $^ $(LFLAGS)
	$(info ${OBJ})

################################### Wildcard Rules ###################################
%.o: %.c 
	@$(CC) -o $@ -c $< $(CFLAGS)

%.o: %.s
	@$(AS) $^ -o $@


################################### Phony Targets ####################################
.PHONY: clean
clean:
	@find . -type f -name '*.o' -delete
	