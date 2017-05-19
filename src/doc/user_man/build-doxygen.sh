#!/bin/bash

set -ep

echo "Building Doxygen"
cd ../doxygen_ref_man
doxygen Doxyfile_RTD
mkdir -p ../user_man/_static/
cp -r html ../user_man/_static/doxygen
breathe-apidoc ./xml -o ../user_man/refman

# Weird bug, this is a workaround
find ../user_man/refman -type f -exec grep -q 'Namespace std' '{}' \; -delete
