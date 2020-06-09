#!/bin/sh
set -e

# run build script
. ./build.sh

# set up isodir
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

# Move compiled kernel into the isodir boot directory
cp sysroot/boot/myos.kernel isodir/boot/myos.kernel

# Create the GRUB config for the kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.kernel
}
EOF

# Run grub-mkrescue to create the cdrom iso
grub-mkrescue -o myos.iso isodir
