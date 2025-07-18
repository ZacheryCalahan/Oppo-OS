C_SOURCES = $(wildcard *.c libc/*.c)
S_SOURCES = $(wildcard *.s)
OBJ = $(C_SOURCES:.c=.o)

CC = ~/opt/cross-riscv/bin/riscv64-elf-gcc
AS = ~/opt/cross-riscv/bin/riscv64-elf-as
LD = ~/opt/cross-riscv/bin/riscv64-elf-ld
GDB= ~/opt/cross-riscv/bin/riscv64-elf-gdb
OBJCPY= ~/opt/cross-riscv/bin/riscv64-elf-objcopy

CFLAGS = -Wall -Wextra -mcmodel=medany -ffreestanding -g
LFLAGS = -T user.ld -nostdlib -o shell.elf

################################### Build Targets  ###################################

# I'm aware this may be scuffed, I'm wrapping my head around a better way to handle
# stuff like this. Stay tuned!

build: ${OBJ}
	$(LD) $^ $(LFLAGS)
	$(OBJCPY) --set-section-flags .bss=alloc,contents -O binary shell.elf shell.bin
	$(OBJCPY) -I binary -O elf64-littleriscv shell.bin shell.bin.o


################################### Wildcard Rules ###################################
%.o: %.c 
	@$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.s
	@$(AS) $^ -o $@


################################### Phony Targets ####################################
