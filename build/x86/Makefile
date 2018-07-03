include ../common.mak

VPATH = ../../src/x86 ../../src

ARCH_SRCS = x86_ioport.c x86_cons.c x86_mem.c x86_vm.c x86_int.c x86_cpu.c x86_thread.c \
	x86_irq.c x86_timer.c x86_keyb.c x86_ps2.c x86_ata.c
ASM_SRCS = x86_boot_asm.S x86_cpu_asm.S x86_int_asm.S x86_thread_asm.S
ALL_SRCS = $(COMMON_SRCS) $(ARCH_SRCS) $(ASM_SRCS)

KERNEL_OBJS = $(ASM_SRCS:%.S=kernel/%.o) $(COMMON_SRCS:%.c=kernel/%.o) $(ARCH_SRCS:%.c=kernel/%.o)

# Grub requires the kernel to be loaded above 1 MB.  We add 4KB
# to avoid the initial kernel stack, which is placed at exactly 1 MB.
KERNEL_BASE_ADDR = 0x101000
KERNEL_ENTRY = _entry
KERNEL_EXE = kernel/geekos.exe

BOOT_ISO = boot.iso

CC = gcc
TARGET_ARCH = i586-elf
TEST = 0

ifneq ($(shell which $(TARGET_ARCH)-$(CC)),) # try prefered arch
    CC := $(TARGET_ARCH)-$(CC)
    TEST = 1
else ifneq ($(shell which i386-elf-$(CC)),) # try fallback to i386
    CC := i386-elf-$(CC)
    TEST = 2
else # fallback to not using cross compiler
    TEST = 3
endif

# If we're on an x86_64 system, force 32-bit code generation
ifeq ($(shell uname -m),x86_64)
    GCC_FORCE_32BIT = -m32
endif

CFLAGS = -Wall -Werror -fno-builtin -fno-stack-protector
DEFS = -DKERNEL
INC = -I../../include/x86 -I../../include

#CC = $(CROSS)gcc
#CC = /home/daveho/linux/tcc/bin/tcc
#CC = tcc
DEP_CC = $(CC)

kernel/%.o : kernel/%.c
	$(CC) -c $(CFLAGS) $(GCC_FORCE_32BIT) $(DEFS) $(INC) -o kernel/$*.o $<

kernel/%.o : kernel/%.S
	$(CC) -c $(GCC_FORCE_32BIT) $(DEFS) -DASM $(INC) -o kernel/$*.o $<

all : $(BOOT_ISO)

$(KERNEL_EXE) : $(KERNEL_OBJS)
	$(CC) -nostdlib $(GCC_FORCE_32BIT) -Wl,-Ttext,$(KERNEL_BASE_ADDR),-e,$(KERNEL_ENTRY),-M \
		-o $@ $(KERNEL_OBJS) \
		> $(@:%.exe=%.map)

# prefere using grub-mkrescue if it exists since the alternative way
# produces an iso image that did not boot on some architectures
ifneq ($(shell which grub-mkrescue),)
    MK_ISO_COMMAND = \
          cp $(KERNEL_EXE) iso/boot/; \
          cp grub.cfg iso/boot/grub; \
	  grub-mkrescue
else
    MK_ISO_COMMAND = \
	    cp boot/stage2_eltorito iso/boot/grub; \
	    cp $(KERNEL_EXE) iso; \
	    cp cdmenu.lst iso/boot/grub/menu.lst; \
	    mkisofs -R -b boot/grub/stage2_eltorito \
		-no-emul-boot -boot-load-size 4 -boot-info-table
endif

$(BOOT_ISO): $(KERNEL_EXE)
	rm -rf iso
	mkdir -p iso/boot/grub
	$(MK_ISO_COMMAND) -o $@ iso

clean :
	rm -f $(KERNEL_OBJS)
	rm -f $(BOOT_ISO)
	rm -f $(KERNEL_EXE) $(KERNEL_EXE:.exe=.map)
	rm -f depend.mak
	rm -rf iso

depend :
	$(DEP_CC) $(CFLAGS) $(INC) -M `../../scripts/vpath.rb "$(VPATH)" $(ALL_SRCS:%=kernel/%)` \
		| ruby -e '$$stdin.each { |l| print "kernel/" unless l.match("^ "); puts l }' \
		> depend.mak

depend.mak :
	touch $@

include depend.mak
