SHELL = /bin/bash
TARGET_BINARY = main
LDLIBS = -lm -pthread -lwiringPi
CC = $(RPI_GCC)
CFLAGS = -Wall -O3
OBJ_DIR = ./objs
LOGGER_DIR = ./logger/src
OBJECTS = $(addprefix $(OBJ_DIR)/, log.o process_command.o common_data.o gpio_setup.o hw_if.o app.o)

.DELETE_ON_ERROR:

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(LOGGER_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

${TARGET_BINARY}: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET_BINARY) $(CFLAGS) $(LDLIBS) $(LDFLAGS)

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

log.o: log.h

app.o: gpio_setup.h common_data.h defines.h hw_if.h process_command.h

common_data.o: common_data.h defines.h

gpio_setup.o: gpio_setup.h defines.h

hw_if.o: hw_if.h defines.h common_data.h

process_command.o: defines.h common_data.h hw_if.h
