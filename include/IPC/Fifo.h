#ifndef FIFO_H
#define FIFO_H

#include <sys/stat.h>
#include <unistd.h>

namespace FIFO {
	bool init();
	void remove();
	
	int writeData(int id, int size, int k);
	int readData(int id, int size);
}

#endif
