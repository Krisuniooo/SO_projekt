#include "../../include/IPC/Semaphore.h"
#include "../../include/Config.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

static int sem_id = -1;

bool SEMAPHORE::init() {
	bool setupSuccess = SEMAPHORE::setupKeyFile();
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return SEMAPHORE::create();
}

bool SEMAPHORE::create() {
	sem_id = SEMAPHORE::getID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(sem_id == -1) return false;
	
	return true;
}

int SEMAPHORE::getID(int flags) {
	if(sem_id != -1) return sem_id;

	key_t key = ftok(SEMAPHORE_KEY_PATH, SEMAPHORE_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	sem_id = semget(key, static_cast<int>(SemaphoreTypes::SEM_COUNT), flags);
	
	if(sem_id == -1) {
		std::cerr << "semget Error: " << strerror(errno) << "\n";
	}
	
	return sem_id;
}


bool SEMAPHORE::lock(int sem_num, bool trylock) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;

	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = -1;
	sop.sem_flg = (trylock ? IPC_NOWAIT : 0);
	
	if(semop(sem_id, &sop, 1) == -1) {
		if(trylock) {
			if(errno == EAGAIN) 
				return false;
			if(errno != EINTR && errno != EIDRM)
				std::cerr << "semop Error: " << strerror(errno) << "\n";
			return false;
		} else {
			std::cerr << "semop Error: " << strerror(errno) << "\n";
			return false;
		}
	}
	
	return true;
}

bool SEMAPHORE::unlock(int sem_num) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;

	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = 1;
	sop.sem_flg = 0;
	
	if(semop(sem_id, &sop, 1) == -1) {
		std::cerr << "semop Error: " << strerror(errno) << "\n";
		return false;
	}
	
	return true;
}

void SEMAPHORE::setValue(int sem_num, int new_val) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return;
	
	union semun {
            int val;
            struct semid_ds *buf;
            unsigned short *array;
        } arg;
        
        arg.val = new_val;

	int val = semctl(sem_id, sem_num, SETVAL, arg);
	if(val == -1) {
		std::cerr << "semctl Error (SETVAL): " << strerror(errno) << "\n";
	}
}

int SEMAPHORE::getValue(int sem_num) {
	if(sem_id == -1 && SEMAPHORE::getID() == -1) return false;
	
	int value = semctl(sem_id, sem_num, GETVAL);
	if(value == -1) {
		std::cerr << "semctl Error (GETVAL): " << strerror(errno) << "\n";
	}
	
	return value;
}

bool SEMAPHORE::setupKeyFile() {
	std::ofstream file(SEMAPHORE_KEY_PATH, std::ios::app);
	
	if(!file.is_open()) 
		return false;
		
	file.close();
	return true;
}
