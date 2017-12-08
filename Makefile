#PATH DEL MAKEFILE
MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

#PATH A LA BASE DEL FIRMWARE
BASE_PATH = $(abspath $(MAKEFILE_DIR)../../)/

#PATH A LA BASE DE LA RUTA A COMMON
TESTCOMMON_PATH = $(BASE_PATH)conformity_tests/common/

#PATH A LA BASE DE LAS LIBS
LIBS_PATH = $(BASE_PATH)libs/

#PATH A LA BASE DE LAS LIBS EXTERNAL
LIBS_EXTERNAL_PATH = $(LIBS_PATH)external/

#PATH A LA BASE DEL PROYECTO PRINCIPAL
MEMO_FW_PATH = $(BASE_PATH)modem_manager/

#PATH EN DONDE QUEDAN LOS ARCHIVOS OBJETO
OUT_PATH = $(MAKEFILE_DIR)out/

#includes all the project definitions
include project.mk

#flags que se le pasa al compilador
CFLAGS += $(foreach inc, $(INC_FILES), -I$(inc) )
CFLAGS += -D_WIN32 -ggdb -Wall

#flags que se le pasa al linker  (pueden agregarse en project.mk)
LFLAGS +=

#regla que linkea luego de tener los requisitos
all: info objs   
	gcc $(OUT_PATH)*.opp -o $(OUT_PATH)$(PROJECT_NAME) $(LFLAGS)

#regla que compila cada .c por serparado en un .o
%.o: %.c
	@echo
	@echo "*** Compilando $< ***"
	@echo
	@mkdir -p $(OUT_PATH)
	@gcc -c $< -Wall $(CFLAGS) -o $(OUT_PATH)$(notdir $(patsubst %.c,%.opp, $< ))


#regla que compila cada .cpp por serparado en un .o
%.opp: %.cpp
	@echo
	@echo "*** Compilando $< ***"
	@echo
	@mkdir -p $(OUT_PATH)
	@g++ -c $< -Wall $(CFLAGS) -o $(OUT_PATH)$(notdir $(patsubst %.cpp,%.opp, $< ))

	
#regla que como prerequisito necesita tener todos los .o (que se compilan por separado).
objs:   $(foreach src_file, $(SRC_FILES),  $(patsubst %.cpp,%.opp, $(src_file) ) ) 

#regla que imprime simplemente la informacion
info:
	@echo '---------------------------------------------------------------------'
	@echo 'Makefile Path:            '$(MAKEFILE_DIR)
	@echo 'Output Path:              '$(OUT_PATH)
	@echo '---------------------------------------------------------------------'
	@echo ''
	
clean:
	@rm -rf $(OUT_PATH)*.o $(OUT_PATH)$(PROJECT_NAME)
	@rm -rf $(OUT_PATH)
