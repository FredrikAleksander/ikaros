#!/bin/sh
set -e
. ./build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/ikaros.kernel isodir/boot/ikaros.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "ikaros" {
	multiboot2 /boot/ikaros.kernel printk.debug=1
}
EOF
grub-mkrescue -o ikaros.iso isodir
