#!/bin/sh

# Exit immediately if command exits with nonzero status
set -e

# Run config.sh from current shell, so we can use the
# environment vars SYSROOT, SYSTEM_HEADER_PROJECTS, DESTDIR
. ./config.sh

# Make a new directory with value SYSROOT
# e.g. ~/myos/sysroot/
mkdir -p "$SYSROOT"

# For each project (kernel, libc, etc.)
# Install their headers (copy) into the sysroot directory
for PROJECT in $SYSTEM_HEADER_PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install-headers)
done
