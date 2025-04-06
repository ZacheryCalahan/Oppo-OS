C_SOURCES = $(wildcard kernel/*.c drivers/*.c klib/*.c)
S_SOURCES = $(wildcard kernel/*.s boot/*.s)
OBJ = $(C_SOURCES:.c=.o)

CC = ~/opt/cross-riscv/bin/riscv64-elf-gcc
AS = ~/opt/cross-riscv/bin/riscv64-elf-as
LN = ~/opt/cross-riscv/bin/riscv64-elf-ld
GDB= ~/opt/cross-riscv/bin/riscv64-elf-gdb
OBJCPY= ~/opt/cross-riscv/bin/riscv64-elf-objcopy

CFLAGS = -Wall -Wextra -mcmodel=medany -ffreestanding -g
LFLAGS = -T linker.ld -nostdlib -o kernel.elf

QEMUOPTS = -machine virt -bios none
QEMUOPTS += -device virtio-gpu-pci -m 4G # 4 Gibibytes of RAM
QEMUOPTS += -monitor mon:stdio


generate_dts:
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -machine dumpdtb=riscv64-virt.dtb
	@dtc -I dtb -O dts riscv64-virt.dtb -o riscv64-virt.dts
	@rm riscv64-virt.dtb
	@mv riscv64-virt.dts misc/riscv64-virt.dts

debug: kernel.bin
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.bin -s -S &
	$(GDB) kernel.elf 


run: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf 

run_sifive_u: kernel.elf
	@echo "Running QEMU for SiFive Unleashed board"
	@qemu-system-riscv64 -machine sifive_u -bios none -kernel kernel.elf -serial mon:stdio

build: kernel.elf
	@echo "Built!"

kernel.bin: ${OBJ} boot/entry.o kernel/sys.o
	@$(LN) $^ $(LFLAGS)
	$(info ${OBJ})
	$(OBJCPY) --only-keep-debug kernel.elf kernel.sym
	$(OBJCPY) -O binary kernel.elf kernel.bin

kernel.elf: ${OBJ} boot/entry.o kernel/sys.o
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
	