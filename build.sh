#!/bin/sh

# Exit immediately if a command exits with a non-zero status
set -e

# Run the headers script
# the '.' is a synonym for "source", aka execute commands
# from a file in the current shell. Environment variables etc
# that are set will remain so after the script runs
# So if headers (and its own sub scripts) set environment, that
# will be available here - PROJECTS, DESTDIR, SYSROOT
. ./headers.sh

# For each project (kernel, libc, etc)
# Enter that directory, set destdir to sysroot, make install
for PROJECT in $PROJECTS; do
  (cd $PROJECT && DESTDIR="$SYSROOT" $MAKE install)
done
