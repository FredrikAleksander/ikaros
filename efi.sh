#!/bin/sh
set -e
. ./build.sh

mkdir -p efidir
mkdir -p efidir/EFI/BOOT
mkdir -p efidir/boot/grub

cp sysroot/boot/ikaros.kernel efidir/EFI/BOOT/ikaros.kernel
cat > grub.tmp.cfg << EOF
insmod part_msdos
configfile (hd0,msdos1)/boot/grub/grub.cfg
EOF
cat > efidir/boot/grub/grub.cfg << EOF
insmod part_msdos
set root=(hd0,msdos1)
set timeout=0
set default=0
menuentry "ikaros" {
	multiboot2 /boot/ikaros.kernel printk.debug=1
	boot
}
EOF
grub-mkstandalone -O i386-pc -o efidir/EFI/BOOT/BOOT.EFI "boot/grub/grub.cfg=grub.tmp.cfg"
