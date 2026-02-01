#include "../../include/IPC/Fifo.h"
#include "../../include/Config.h"
#include "../../include/Utils.h"

bool FIFO::init() {
	for(int i=0; i<PRODUCTS; i++) {
		std::string path = FIFO_PATH + std::to_string(i);
		unlink(path.c_str());
		if(mkfifo(path.c_str(), 0600) == -1) {
			perror("mkfifo error");
			return false;
		}
	}
	return true;
}

void FIFO::remove() {
	for(int i=0; i<PRODUCTS; i++) {
		std::string path = FIFO_PATH + std::to_string(i);
		unlink(path.c_str());
	}
}

int FIFO::writeData(int id, int size, int k) {
	std::string path = FIFO_PATH + std::to_string(id);
	int fd = open(path.c_str(), O_RDWR);
	if(fd == -1) return -1;
	char data[size];
	memset(data, 'v', size);
	
	int written = 0;
	for(int i = 0; i<k; i++) {
		
		ssize_t bytes = write(fd, data, size);
		
		if(bytes == -1) {
			perror("fifo write error");		
			break;
		} else {
			written += write(fd, data, size);
		}
	}
		
	close(fd);
	return written;
}

int FIFO::readData(int id, int _size) {
	std::string path = FIFO_PATH + std::to_string(id);
	int fd = open(path.c_str(), O_RDWR);
	if(fd == -1) return -1;
	char data[_size];
	int bytes = read(fd, data, _size);
	
	close(fd);
	
	
	return bytes;
}
