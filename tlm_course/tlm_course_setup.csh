#! /bin/csh -fx

# tlm course setup script for compilation with synopsis tools
#     Edit this file to reflect your SystemC compilation settings
#     Only tested on mammut
# Copyright (C) 2012 imec

echo ""
echo "-- setup script ---------------------"
echo ""

echo "   * compiler "
setenv PATH /imec/other/bee/bin/gcc-4.3.6:$PATH

echo "   * SystemC and TLM2"
setenv SYSTEMC_HOME /imec/other/bee/public/tools/systemc/systemc-2.3.0_gcc-4.3.6
setenv LIB_ARCH linux64 #for 64-bit compiler
#setenv LIB_ARCH linux #for 32-bit compiler
echo "-----------------------------------------"
echo ""

