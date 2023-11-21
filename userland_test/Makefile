# Makefile for CUDA and C++ code

# Compiler settings
CC = g++
NVCC = nvcc
CUDA_PATH = /usr/local/cuda
CFLAGS = -std=c++11 -I$(CUDA_PATH)/include
LDFLAGS = -lcudart -L$(CUDA_PATH)/lib64

# File names
EXEC = userland_test
SOURCES = test.cpp
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LDFLAGS)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
