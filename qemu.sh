#!/bin/sh
set -e

# create the cdrom iso for our kernel
. ./iso.sh

# Run qemu to test the iso
qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso
