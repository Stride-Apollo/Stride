#!/bin/bash
cd build/installed

./bin/gtester --gtest_output=xml --gtest_color=yes --gtest_filter=-HDF5Scenario* > gtest_output.txt 2>&1
EXIT_CODE=$?

echo "
╔════════════════════════════════════╗
║              Overview              ║
╚════════════════════════════════════╝"
../../src/test/print-gtest.py test_detail.xml


echo "
╔════════════════════════════════════╗
║               Output               ║
╚════════════════════════════════════╝"
cat gtest_output.txt

cd ../..
exit $EXIT_CODE
