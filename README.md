# Torment-OS
Welcome to my hobby OS that I'm writing for the RISC-V architecture using qemu's virt board. The name is a joke on my mental state while writing drivers with little documentation.

# Build
1. Build a cross compiler for the `riscv64-elf` target with GCC version 14.2.0 and binutils version 2.44 (The OS is STRICTLY for 64bit, 32bit will not work)
2. Install qemu-system-riscv64
3. Clone this repository
4. Update the Makefile with the location to your cross compiler via the CC, AS, and LN variables.
5. Run `make run` to compile, link, and run the binary in qemu





