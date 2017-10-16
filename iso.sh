#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/ikaros.kernel isodir/boot/ikaros.kernel
cat > isodir/boot/grub/grub.cfg << EOF
insmod all_video
gfxpayload="keep"
terminal_output gfxterm
#set timeout=0
set default=0

menuentry "ikaros" {
	multiboot2 /boot/ikaros.kernel printk.debug=1
	#boot
}
EOF
GRUB_TERMINAL_OUTPUT="gfxterm" grub-mkrescue -o ikaros.iso isodir
