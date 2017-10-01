#!/bin/sh
#gdb -ex "target remote :1234" -ex "add-symbol-file kernel/ikaros.kernel 0x00100000" # For early init debugging
gdb kernel/ikaros.kernel -ex "target remote :1234" # For kernel debugging
