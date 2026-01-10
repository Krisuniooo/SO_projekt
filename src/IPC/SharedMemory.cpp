#include "../../include/IPC/SharedMemory.h"
#include "../../include/Config.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

static int shm_id = -1;
static void* shm_ptr = nullptr;

bool SHAREDMEMORY::init() {
	bool setupSuccess = SHAREDMEMORY::setupKeyFile();
	
	if(setupSuccess == false) {
		std::cerr << "Could not create key file\n";
		return false;
	}
	
	return SHAREDMEMORY::create();
}

bool SHAREDMEMORY::create() {
	int block_id = SHAREDMEMORY::getID(IPC_CREAT | IPC_EXCL | 0600);
	
	if(block_id == -1) {
		std::cerr << "Shared Memory IPC_CREAT Error\n";
		return false;
	}
	
	return true;
}

// Default flags: 0600
int SHAREDMEMORY::getID(int flags) {
	if(shm_id != -1) return shm_id;

	key_t key = ftok(SHARED_MEM_KEY_PATH, SHARED_MEM_KEY);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		return -1;
	}
	
	shm_id = shmget(key, sizeof(SharedData), flags);
	
	if(shm_id == -1) {
		std::cerr << "shmget Error: " << strerror(errno) << "\n";
	}
	
	return shm_id;
}

void* SHAREDMEMORY::attach() {
	if(shm_ptr != nullptr) return shm_ptr;
	if(shm_id == -1 && SHAREDMEMORY::getID() == -1) return nullptr;
	shm_ptr = shmat(shm_id, nullptr, 0);

	if(shm_ptr == (void*)-1) {
		std::cerr << "shmat Error: " << strerror(errno) << "\n";
		shm_ptr = nullptr;
	}
	return shm_ptr;
}

bool SHAREDMEMORY::detach() {
	if(shm_ptr == nullptr || shm_ptr == (void*)-1) return true;

	if(shmdt(shm_ptr) == -1) {
		std::cerr << "shmdt Error: " << strerror(errno) << "\n";
		return false;
	}
	
	shm_ptr = nullptr;
	return true;
}

void SHAREDMEMORY::destroy() {
	if(shm_id == -1) return;
	
	if(shmctl(shm_id, IPC_RMID, nullptr) == -1) {
		std::cerr << "shmctl Error (delete): " << strerror(errno) << "\n";
	}
	shm_id = -1;
}

bool SHAREDMEMORY::setupKeyFile() {
	std::ofstream file(SHARED_MEM_KEY_PATH, std::ios::app);
	
	if(!file.is_open()) 
		return false;
		
	file.close();
	return true;
}
