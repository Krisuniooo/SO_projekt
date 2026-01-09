#include "../include/Shared.h"
#include "../include/Product.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <sys/sem.h>

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

SharedMemManager::SharedMemManager(const std::string& path, char id, size_t size, bool create) : is_owner(create) {
	if(setupKeyFile(path) == false) {
		std::cerr << "Could not create nor access key file: " << strerror(errno) << "\n";
		exit(1);
	}
	

	key_t key = ftok(path.c_str(), id);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		exit(1);
	}
	

	int flags = (create ? IPC_CREAT | IPC_EXCL | 0666 : 0666);
	shm_id = shmget(key, size, flags);

	if(shm_id == -1) {
		std::cerr << "shmget Error: " << strerror(errno) << "\n";
		exit(1);
	}
	

	shm_addr = shmat(shm_id, nullptr, 0);

	if(shm_addr == (void*)-1) {
		std::cerr << "shmat Error: " << strerror(errno) << "\n";
		shm_addr = nullptr;
		exit(1);
	}
}

SharedMemManager::~SharedMemManager() {
	if(!shm_addr) return;

	if(shmdt(shm_addr) == -1) {
		std::cerr << "shmdt Error: " << strerror(errno) << "\n";
		exit(1);
	}
	shm_addr = nullptr;

	if(!is_owner) return;

	if(shmctl(shm_id, IPC_RMID, nullptr) == -1) {
		std::cerr << "shmctl Error (delete): " << strerror(errno) << "\n";
		exit(1);
	}
}

int SharedMemManager::getSize() const {
	return sizeof(SharedData);
}

int SharedMemManager::getID() const {
	return shm_id;
}

void* SharedMemManager::getAddress() const {
	return shm_addr;
}


Semaphore::Semaphore(const std::string& path, char id, int init_val){
	key_t key = ftok(path.c_str(), id);
	
	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		exit(1);
	}
	
	// ...
}

void Semaphore::waitSem(int sem_num) {
	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;
	
	if(semop(sem_id, &sop, 1) == -1) {
		std::cerr << "semop Error: " << strerror(errno) << "\n";
	}
}

void Semaphore::signalSem(int sem_num) {
	struct sembuf sop;
	
	sop.sem_num = sem_num; 
	sop.sem_op = 1;
	sop.sem_flg = SEM_UNDO;
	
	if(semop(sem_id, &sop, 1) == -1) {
		std::cerr << "semop Error: " << strerror(errno) << "\n";
	}
}

void Semaphore::writeSem(int sem_num, int new_val) {
	int val = semctl(sem_id, sem_num, SETVAL, new_val);
	if(val == -1) {
		std::cerr << "semctl Error (SETVAL): " << strerror(errno) << "\n";
	}
}

int Semaphore::readSem(int sem_num) {
	int val = semctl(sem_id, sem_num, GETVAL);
	if(val == -1) {
		std::cerr << "semctl Error (GETVAL): " << strerror(errno) << "\n";
	}
	return val;
}


bool setupKeyFile(const std::string& path) {
	std::ofstream file(path, std::ios::app);
	
	if(!file.is_open()) 
		return false;
		
	file.close();
	return true;
}
