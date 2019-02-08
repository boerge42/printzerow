DIR_FONTS = ./Fonts
DIR_OBJ = ./obj
DIR_BIN = ./bin
DIR_INSTALL=/usr/bin

OBJ_C = $(wildcard ${DIR_FONTS}/*.c ${DIR_OBJ}/*.c)
OBJ_O = $(patsubst %.c,${DIR_BIN}/%.o,$(notdir ${OBJ_C}))

TARGET = printzerow_gui

CC = gcc

DEBUG = -g -O0 -Wall
CFLAGS += $(DEBUG)

LIB = -lwiringPi -lm -lmosquitto -ljson-c

${TARGET}:${OBJ_O}
	$(CC) $(CFLAGS) $(OBJ_O) -o $@ $(LIB)

${DIR_BIN}/%.o : $(DIR_OBJ)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ $(LIB)

${DIR_BIN}/%.o:$(DIR_FONTS)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@ 

install:
	strip $(TARGET)
	cp $(TARGET) $(DIR_INSTALL)/
	chown root:root $(DIR_INSTALL)/$(TARGET)
	chmod 0755 $(DIR_INSTALL)/$(TARGET)
	
clean :
	$(RM) $(DIR_BIN)/*.* $(TARGET) $(DIR_OBJ)/.*.sw?
