#include "../include/IPC.h"
#include <iostream>

bool IPC::init() {
	bool shm_initialized = SHAREDMEMORY::init();
	//Log
	if(shm_initialized) {
		std::cout << "\n\tSHAREDMEMORY initialized\n";
	} else {
		std::cout << "\n\tSHAREDMEMORY initialization failed\n";
		return false;
	}
	
	bool sem_initialized = SEMAPHORE::init();
	//Log
	if(sem_initialized) {
		std::cout << "\n\tSEMAPHORE initialized\n";
	} else {
		std::cout << "\n\tSEMAPHORE initialization failed\n";
		return false;
	}
	
	return true;
}
