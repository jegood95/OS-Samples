# the compiler: gcc for C programs, define as g++ for C++
CC = gcc

#compiler flags:
# -g adds debugging information to the executable file
# -Wall turns on most, but not all, compiler warnings
# -std=c99 to define the '99 standard for C programming
CFLAGS = -g -Wall -std=c99 -lpthread -lm
 
# the build target executable
TARGET = p3_c
TARGET1 = p3_s

all: $(TARGET) $(TARGET1)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o client $(TARGET).c

$(TARGET1): $(TARGET1).c
	$(CC) $(CFLAGS) -o server $(TARGET1).c

clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET1)