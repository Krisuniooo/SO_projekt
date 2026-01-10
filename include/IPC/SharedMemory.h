#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

namespace SHAREDMEMORY {
	bool init();
	
	bool create();
	int getID(int flags = 0600);
	void* attach(); 
	bool detach();
	void destroy();
	bool setupKeyFile();
	
}

#endif
