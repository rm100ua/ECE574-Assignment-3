#!/bin/sh

# Check to see if binary exists

if [ ! -f ././../../cmake-build-debug/src/hlysn ]; then
    echo "File doesn't exist!, exiting"
    exit 1
fi

# prepare for GNU Parallel
# Will get all files under resources
# DO NOT UNCOMMENT DOES NOT REGENERATE CORRECT FORMAT
#find ././../ -name *.c -print0 | xargs -0 -n1 > commands


# REQUIRES GNU Parallel
cat commands | parallel -j+0
