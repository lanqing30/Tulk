
C_SOURCES = $(shell find . -name "*.c")
C_OBJECTS = $(patsubst %.c, %.o, $(C_SOURCES))
S_SOURCES = $(shell find . -name "*.s")
S_OBJECTS = $(patsubst %.s, %.o, $(S_SOURCES))
GAS_SOURCES = $(shell find . -name "*.asm")
GAS_OBJECTS = $(patsubst %.asm, %.o, $(GAS_SOURCES))

CC = gcc
LD = ld
ASM = nasm
GAS = as

C_FLAGS = -c -Wall -m32 -ggdb -gstabs+ -nostdinc -fno-builtin -fno-stack-protector -I include
LD_FLAGS = -T scripts/kernel.ld -m elf_i386 -nostdlib
ASM_FLAGS = -f elf -g -F stabs
GAS_FLAGS = --32
OUT = tulk

# http://stackoverflow.com/questions/9233447/what-is-the-makefile-target-c-o-for

all:  $(S_OBJECTS) $(C_OBJECTS) $(GAS_OBJECTS) link update_image

%.o:%.s
	@echo compiling the Intel_assembly_source_files $< ...
	$(ASM) $(ASM_FLAGS) $<

%.o:%.c
	@echo compiling the c_source_files $< ...
	$(CC) $(C_FLAGS) $< -o $@

%.o:%.asm 
	@echo compiling the att_assembly_source_files $< ...
	$(GAS) $(GAS_FLAGS) $< -o $@

link:
	@echo linking the kernel files $< ...
	$(LD) $(LD_FLAGS) $(GAS_OBJECTS) $(S_OBJECTS) $(C_OBJECTS) -o $(OUT)

.PHONY:clean
clean:
	$(RM) $(GAS_OBJECTS) $(S_OBJECTS) $(C_OBJECTS) $(OUT)

.PHONY:update_image
update_image:
	sudo mount floppy.img /mnt/kernel
	sudo cp $(OUT) /mnt/kernel/$(OUT)
	sleep 1
	sudo umount /mnt/kernel

.PHONY:mount_image
mount_image:
	sudo mount floppy.img /mnt/kernel

.PHONY:umount_image
umount_image:
	sudo umount /mnt/kernel

.PHONY:qemu
qemu:
	qemu -fda floppy.img -boot a -m 128M

.PHONY:bochs
bochs:
	bochs -f scripts/bochsrc.txt

.PHONY:debug
debug:
	qemu -S -s -fda floppy.img -boot a &
	sleep 1
	cgdb -x scripts/gdbinit
