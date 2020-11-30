#!/bin/sh
set -e

# run build script
. ./build.sh

# set up isodir
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

# Move compiled kernel into the isodir boot directory
# cp sysroot/boot/myos.kernel isodir/boot/myos.kernel
cp sysroot/boot/myos.bin isodir/boot/myos.bin

# Create disk image from template, add kernel & grub config
cp template.img myos_disk.img
#FREE_LOOP=$(losetup -f)
#echo "Free device: ${FREE_LOOP}"
#sudo losetup -f myos_disk.img -o 1048576
#sudo mount $FREE_LOOP /mnt

# Mount image at the filesystem offset
sudo mount -o offset=1048576 -t vfat myos_disk.img /mnt

# Create the GRUB config for the kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "myos" {
	multiboot /boot/myos.bin
}
EOF


# copy files to file system disk partition
sudo cp sysroot/boot/myos.bin /mnt/boot/myos.bin
sudo cp isodir/boot/grub/grub.cfg /mnt/boot/grub/grub.cfg

# Run grub-mkrescue to create the iso
grub-mkrescue -o myos.iso isodir

# now unmount the image
sudo umount /mnt
