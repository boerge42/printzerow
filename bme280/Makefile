SRC = $(wildcard *.c)
HDR = $(wildcard *.h)
TARGET = bme280mqtt
CC := gcc
CFLAGS = -Wall
CFLAGS += -Os -g -Wextra -pedantic
CLIBS  += -lmosquitto -lwiringPi -lm

$(TARGET): $(patsubst %.c, %.o, $(SRC))
	$(CC) $(CFLAGS) -o $(TARGET) $^ $(CLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

install:
	strip $(TARGET)
	cp $(TARGET) /usr/bin/
	chown root:root /usr/bin/$(TARGET)
	chmod 0755 /usr/bin/$(TARGET)
	
clean:
	rm -f *.o *~ $(TARGET) Makefile.dep

Makefile.dep: $(SRC) $(HDR)
	$(CC) -MM *.c > Makefile.dep

-include Makefile.dep
