# the compiler: nvcc for CUDA programs
CC = nvcc
 
# the build target executable
TARGET = p2
TARGET1 = p3

all: $(TARGET) $(TARGET1)

$(TARGET): $(TARGET).cu
	$(CC) -o $(TARGET) $(TARGET).cu

$(TARGET1): $(TARGET1).cu
	$(CC) -lm -o $(TARGET1) $(TARGET1).cu

clean:
	$(RM) $(TARGET)
	$(RM) $(TARGET1)