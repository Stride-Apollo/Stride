#!/bin/bash

set -ep

echo "Building Doxygen"
cd ../doxygen_ref_man
doxygen Doxyfile_RTD
mkdir -p ../user_man/_static/
cp -r html ../user_man/_static/doxygen
