PROJECT_NAME = hex2array

INC_FILES += $(MAKEFILE_DIR)hex2array
   
SRC_FILES += $(MAKEFILE_DIR)hex2array/hex2array.cpp
SRC_FILES += $(MAKEFILE_DIR)hex2array/line.cpp
 
#INDICO QUE SE VA A DEFINIR COMPILACIONES CONDICIONALES PARA REALIZAR TESTS

#CFLAGS += -std=gnu99
LFLAGS += -lstdc++
