#include "../include/Shared.h"
#include "../include/Product.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <cerrno>
#include <fstream>

SharedMemManager::SharedMemManager(const std::string& path, char id, size_t size, bool create) : is_owner(create) {

	if(create) {
		std::cout << "\t" << path << "\n";
		std::ofstream file(path);
		if(!file) {
			std::cerr << "Could not create key file\n";
			return;
		}
		file.close();
	} else {
		std::ifstream file(path);
		if(!file) {
			std::cerr << "key file does not exist\n";
			return;
		}
		file.close();
	}

	key_t key = ftok(path.c_str(), id);

	if(key == -1) {
		std::cerr << "ftok Error: " << strerror(errno) << "\n";
		exit(1);
	}

	int flags = (create ? IPC_CREAT | IPC_EXCL | 0666 : 0666);
	//std::cout << "\t " << key << " " << size << " " << flags << "\n";
	shm_id = shmget(key, size, flags);

	if(shm_id == -1) {
		std::cerr << "shmget Error (create): " << strerror(errno) << "\n";
		exit(1);
	}
	std::cout << "\t " << key << "\n";

	shm_addr = shmat(shm_id, nullptr, 0);

	if(shm_addr == (void*)-1) {
		std::cerr << "shmat Error: " << strerror(errno) << "\n";
		shm_addr = nullptr;
		exit(1);
	}
}

SharedMemManager::~SharedMemManager() {
	if(!shm_addr) return;

	shmdt(shm_addr);
	shm_addr = nullptr;


	if(!is_owner) return;

	if(shmctl(shm_id, IPC_RMID, nullptr) == -1) {
		std::cerr << "shmctl Error (delete): " << strerror(errno) << "\n";
		exit(1);
	}

	shm_id = -1;
	is_owner = false;
	shm_addr = nullptr;
}

int SharedMemManager::getSize() const {
	return sizeof(SharedData);
}

void* SharedMemManager::getAddress() const {
	return shm_addr;
}
