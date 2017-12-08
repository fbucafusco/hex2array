# hex2array
forked from : https://github.com/meriororen/hex2array

Convert intel hex format to C array.

Useful for compiling binary images from one project into another. 

ie: BOOTLOADER + MAIN APPLICATION

Binary format available at out/hex2array.exe

### Compilation:

make

### Usage:

hex2array `<input hex file>` `<output c/h file>` modifiers

#### Modifiers
`<width>` [optional] data width of each element of the array: 8, 16, 32. Default value is uint8_t
  
### Pending Changes:
- add endiannes modifier
- add output modifiers (# of elements per line, etc)

