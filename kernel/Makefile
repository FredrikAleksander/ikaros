DEFAULT_TARGET:=$(shell ../default-target.sh)
TARGET?=$(DEFAULT_TARGET)
TARGETARCH:=$(shell ../target-triplet-to-arch.sh $(TARGET))

CFLAGS?=-g -Wall
CPPFLAGS?=
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -Werror -DCONFIG_DEBUG_SERIAL
CPPFLAGS:=$(CPPFLAGS) -D__is_kernel -Iinclude -DCONFIG_DEBUG_SERIAL
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

ARCHDIR=arch/$(TARGETARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
fonts/console.o \
kernel/acpi/acpi.o \
kernel/block/block_device.o \
kernel/console/console.o \
kernel/console/fbconsole.o \
kernel/console/null_console.o \
kernel/debug/serial_debug.o \
kernel/input/keyboard.o \
kernel/memory/memory_map.o \
kernel/memory/memory_region.o \
kernel/pci/pci.o \
kernel/scheduler/scheduler.o \
kernel/scheduler/wait.o \
kernel/kernel.o \

KERNEL_TEST_FILES=kernel/tests/kernel_tests.c

OBJS=\
$(ARCHDIR)/kernel/boot/crti.o \
$(ARCHDIR)/kernel/boot/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/kernel/boot/crtend.o \
$(ARCHDIR)/kernel/boot/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/kernel/boot/crti.o \
$(ARCHDIR)/kernel/boot/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/kernel/boot/crtend.o \
$(ARCHDIR)/kernel/boot/crtn.o \

.PHONY: all clean install install-headers install-kernel test
.SUFFIXES: .o .c .S .psf

all: ikaros.kernel

ikaros.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	@$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	@grub-file --is-x86-multiboot2 ikaros.kernel

kernel.test: $(KERNEL_TEST_FILES)
	@$(HOST_CC) $(CFLAGS) $(CPPFLAGS) -I../unittest/include -std=gnu11 $(KERNEL_TEST_FILES) -o $@

test: kernel.test
	@clear
	@./kernel.test

$(ARCHDIR)/kernel/boot/crtbegin.o $(ARCHDIR)/kernel/boot/crtend.o:
	@OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	@$(CC) -MD -c $< -o $@ -std=gnu99 $(CFLAGS) $(CPPFLAGS)

.S.o:
	@$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)
.psf.o:
	@objcopy -I binary -O elf32-i386 -B i386 --rename-section .data=.rodata,alloc,load,readonly,data,contents $< $@

clean:
	@rm -f ikaros.kernel
	@rm -f kernel.test
	@rm -f $(OBJS) *.o */*.o */*/*.o
	@rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	@mkdir -p $(DESTDIR)$(INCLUDEDIR)
	@cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
	@cp -R --preserve=timestamps $(ARCHDIR)/include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: ikaros.kernel
	@mkdir -p $(DESTDIR)$(BOOTDIR)
	@cp ikaros.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
