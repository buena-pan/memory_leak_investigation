#include <iostream>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cuda_runtime.h>
#include <stdlib.h>
using namespace std;

int main() {
	std::string filename = "/dev/simple_device";
	size_t file_size = 4*1024*124;
	int fd = open(filename.c_str(), O_RDONLY);
	//int fd = open(filename.c_str(), O_RDWR);
	//char* pointer_to_mmap_zone = (char*)(mmap(nullptr, file_size, PROT_READ | PROT_WRITE , MAP_SHARED, fd, 0));
	char* pointer_to_mmap_zone = (char*)(mmap(nullptr, file_size, PROT_READ , MAP_PRIVATE, fd, 0));
	cudaError_t registerStatus = cudaHostRegister(pointer_to_mmap_zone,file_size,  cudaHostRegisterDefault );
	cudaError_t unregisterStatus = cudaHostUnregister(pointer_to_mmap_zone); //cudaHostRegister+0x1cd in CUDA shared library
	munmap(pointer_to_mmap_zone, file_size);
	close(fd);
	return 0;
}

