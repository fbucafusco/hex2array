# hex2array
forked from : https://github.com/meriororen/hex2array

Convert intel hex format to C array.

Useful for compiling binary images from one project into another. 

ie: BOOTLOADER + MAIN APPLICATION

Binary format available at out/hex2array.exe

### Compilation:

make

### Usage:

hex2array `<input hex file>` `<output c/h file>` <modifiers>

#### Modifiers
-wXX XX = data width of each element of the array: 8, 16, 32
-b      = adds pradding (0xFF) from the first section to the first data
-cX  X  = target compiler selection 0: none
                                    1: gcc
									2: armcc 
									3: iar
									
### Pending Changes:
- add endiannes modifier
- add output modifiers (# of elements per line, etc)
- support for gcc / iar
- -b modifier generates an ugly non aligned output format. 