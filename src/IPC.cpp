#include "../include/IPC.h"
#include <iostream>

bool IPC::init() {
	bool shm_initialized = SHAREDMEMORY::init();
	//Log
	if(!shm_initialized) {
		std::cout << "\n\tSHAREDMEMORY initialization failed\n";
		return false;
	}
	
	bool sem_initialized = SEMAPHORE::init();
	//Log
	if(!sem_initialized) {
		std::cout << "\n\tSEMAPHORE initialization failed\n";
		return false;
	}
	
	bool mq_initialized = MESSAGEQUEUE::init();
	if(!mq_initialized) {
		std::cout << "\n\tMESSAGEQUEUE initialization failed\n";
		return false;
	}
	
	std::cout << "\t\t" << shm_initialized << sem_initialized << mq_initialized << "\n";
	
	return true;
}

bool IPC::destroyAll() {
	bool a = SHAREDMEMORY::detach();
	bool b = SHAREDMEMORY::destroy();
	bool c = SEMAPHORE::destroy();
	bool d = MESSAGEQUEUE::destroyLogger();
	bool e = MESSAGEQUEUE::destroyRegister();

	return (a && b && c && d && e);
}
