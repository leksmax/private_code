CROSS_COMPILE = 
CC = $(CROSS_COMPILE)gcc
FLAGS = -g
OBJS = main.o 1.o
TARGET = main

.PHONY: all clean build
all: $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(TARGET)
$(OBJS):%.o: %.c
	$(CC) -c $< -o $@
clean:
	RM $(OBJS) $(TARGET)
