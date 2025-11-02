# Oppo-OS
Welcome to my hobby OS that I'm writing for the RISC-V architecture using qemu's virt board.

# Build
1. Build a cross compiler for the `riscv64-elf` target with GCC version 14.2.0 and binutils version 2.44 (The OS is STRICTLY for 64bit, 32bit will not work)
2. Install `qemu-system-riscv64`
3. Clone this repository
4. Update the Makefile with the location to your cross compiler via the CC, AS, and LN variables.
5. Add a disk image named `disk.img` containing a ext2 filesystem to the root directory of the project, as QEMU expects an image file. Alternatively, run `make generate_new_fat32` to generate a fat32 disk image, to satisfy QEMU. The OS will not currently read FAT32 without changes to the kernel, but file system auto-mounting will occur in the future.
6. Run `make run` to compile, link, and run the binary in qemu





