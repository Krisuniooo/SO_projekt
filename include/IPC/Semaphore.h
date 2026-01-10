#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>

namespace SEMAPHORE {
	bool init();
	
	int getID(int flags = 0600);
	bool create();
	bool lock(int sem_num, bool trylock = false);
	bool unlock(int sem_num);
	void setValue(int sem_num, int value);
	int getValue(int sem_num);
	bool destroy();
	bool setupKeyFile();
}

#endif
