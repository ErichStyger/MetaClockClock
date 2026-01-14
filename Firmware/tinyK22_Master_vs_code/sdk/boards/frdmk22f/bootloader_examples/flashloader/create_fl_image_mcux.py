#! /usr/bin/env python

# Copyright (c) 2013-2014 Freescale Semiconductor, Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# o Redistributions of source code must retain the above copyright notice, this list
#   of conditions and the following disclaimer.
#
# o Redistributions in binary form must reproduce the above copyright notice, this
#   list of conditions and the following disclaimer in the documentation and/or
#   other materials provided with the distribution.
#
# o Neither the name of Freescale Semiconductor, Inc. nor the names of its
#   contributors may be used to endorse or promote products derived from this
#   software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
from __future__ import print_function
import sys
import os
import elf

PY = sys.version_info[0]
# usage: create_fl_image.py <elffile> <binfile> <cfile>

def main(argv):
    # Collect arguments
    if len(sys.argv) != 4:
        print('usage: create_fl_image.py <elffile> <binfile> <cfile>')
        sys.exit(2)
    elfFilename = argv[0]
    binFilename = argv[1]
    cFilename = argv[2]

    # Open files
    try:
        binFile = open(binFilename, 'rb')
    except IOError:
        print('cannot open file %s' % binFilename)
        sys.exit(2)
    try:
        cFile = open(cFilename, 'w')
    except IOError:
        print('cannot open file %s' % cFilename)
        sys.exit(2)
    elfData = elf.ELFObject()
    try:
        with open(elfFilename, 'rb') as elfFile:
            elfData.fromFile(elfFile)
            if elfData.e_type != elf.ELFObject.ET_EXEC:
                raise Exception("No executable")
            resetHandler = elfData.getSymbol("Reset_Handler")
            vectors = elfData.getSymbol("__isr_vector")
            stack = elfData.getSymbol("__StackTop")
    except:
        print('cannot process file %s' % elfFilename)
        sys.exit(2)


    # Print header
    print ('/* Created by create_fl_image.py, do not edit */\n', file=cFile)
    print ('#include "bootloader_common.h"\n', file=cFile)
    print ('const uint8_t g_flashloaderImage[] = {', file=cFile)

    # Print byte data
    totalBytes = 0
    while True:
        data = binFile.read(16)
        dataLen = len(data)
        if dataLen == 0: break
        totalBytes += dataLen;
        cFile.write('    ')
        if PY == 2:
            data = bytearray(data)
        for i in range(dataLen):
            cFile.write('0x%02x, ' % data[i])
        print (file=cFile)
    print('};\n', file=cFile)

    # Print size and other info
    cFile.write('const uint32_t g_flashloaderSize = %dU;\n' % totalBytes)
    cFile.write('const uint32_t g_flashloaderBase = 0x%x;\n' % vectors.st_value)
    cFile.write('const uint32_t g_flashloaderEntry = 0x%x;\n' % resetHandler.st_value)
    cFile.write('const uint32_t g_flashloaderStack = 0x%x;\n' % stack.st_value)

if __name__ == "__main__":
   main(sys.argv[1:])
