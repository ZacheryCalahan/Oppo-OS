C_SOURCES = $(wildcard kernel/*.c drivers/*.c klib/*.c arch/*.c)
S_SOURCES = $(wildcard boot/*.s)
OBJ = $(C_SOURCES:.c=.o)

CC = ~/opt/cross-riscv/bin/riscv64-elf-gcc
AS = ~/opt/cross-riscv/bin/riscv64-elf-as
LD = ~/opt/cross-riscv/bin/riscv64-elf-ld
GDB= ~/opt/cross-riscv/bin/riscv64-elf-gdb
OBJCPY= ~/opt/cross-riscv/bin/riscv64-elf-objcopy

CFLAGS = -Wall -Wextra -mcmodel=medany -ffreestanding -g
LFLAGS = -T linker.ld -nostdlib -o kernel.elf

QEMUOPTS =  -machine virt -bios default -m 1G # 1 Gibibytes of RAM
QEMUOPTS += -device virtio-gpu-device,bus=virtio-mmio-bus.1 -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0
QEMUOPTS += -drive id=drive0,file=disk.img,if=none,format=raw # use fat_filesystem as a disk
QEMUOPTS += -monitor mon:stdio
#QEMUOPTS += -d unimp,guest_errors,int,cpu_reset -D qemu.log # Specific to debugging traps, comment when not in use. (MAKES LARGE FILES!)

################################### Build Targets  ###################################
debug: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -s -S &
	$(GDB) kernel.elf 

run: kernel.elf 
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf 

# I'm aware this may be scuffed, I'm wrapping my head around a better way to handle
# stuff like this. Stay tuned!
user/shell.bin.o:
	$(MAKE) -C user -B # Force to remake user apps

build: kernel.elf
	@echo "Built!"

kernel.elf: ${OBJ} boot/entry.o user/shell.bin.o
	@$(LD) $^ $(LFLAGS)
	$(info ${OBJ})

################################### Wildcard Rules ###################################
%.o: %.c 
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	@$(AS) $^ -o $@


################################### Phony Targets ####################################

# Generate a device tree blob
generate_dts: kernel.elf
	@qemu-system-riscv64 $(QEMUOPTS) -kernel kernel.elf -machine dumpdtb=riscv64-virt.dtb
	@dtc -I dtb -O dts riscv64-virt.dtb -o riscv64-virt.dts
	@rm riscv64-virt.dtb
	@mv riscv64-virt.dts misc/riscv64-virt.dts

# Generate a 128MB FAT32 filesystem
generate_new_fat32:
	qemu-img create -f raw disk.img 128M
	mkfs.fat -F 32 disk.img

# Make a backup of the filesystem
backup_file_system:
	cp disk.img misc/disk.img
	echo "Backup created in misc/disk.img"

# Restore backup
restore_backup:
	rm disk.img
	cp misc/disk.img disk.img

# Mount the disk.img to .mnt
mount_disk:
	mkdir -p .mnt
	sudo mount -o loop disk.img .mnt

# Unmount the disk.img from .mnt
unmount_disk:
	sudo umount .mnt
	rm -rf .mnt

# Clean directories of build files
clean:
	@find . -type f -name '*.o' -delete
	@find . -type f -name 'kernel.sym' -delete
	@find . -type f -name '*.elf' -delete
	@find . -type f -name '*.bin' -delete
	@find . -type f -name 'qemu.log' -delete
	