#!/bin/sh
# Run this to generate all the initial makefiles, etc.

# old autoreconf/aclocal versions fail hard if m4 doesn't exist
mkdir -p m4
autoreconf --force --install
echo "Now type './configure ...' and 'make' to compile."
