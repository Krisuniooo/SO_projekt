#include "../include/IPC.h"
#include <iostream>

bool IPC::init() {
	bool shm_initialized = SHAREDMEMORY::init();
	//Log
	if(!shm_initialized) {
		std::cout << "\n\tSHAREDMEMORY initialization failed\n";
		IPC::destroyAll();
		return false;
	}
	
	bool sem_initialized = SEMAPHORE::init();
	//Log
	if(!sem_initialized) {
		std::cout << "\n\tSEMAPHORE initialization failed\n";
		IPC::destroyAll();
		return false;
	}
	
	bool mq_initialized = MESSAGEQUEUE::init();
	if(!mq_initialized) {
		std::cout << "\n\tMESSAGEQUEUE initialization failed\n";
		IPC::destroyAll();
		return false;
	}
	
	bool fifo_initialized = FIFO::init();
	if(!fifo_initialized) {
		std::cout << "\n\tFIFO initialization failed\n";
		IPC::destroyAll();
		return false;
	}
	
	return true;
}

bool IPC::destroyAll() {
	bool a = SHAREDMEMORY::detach();
	bool b = SHAREDMEMORY::destroy();
	bool c = SEMAPHORE::destroy();
	bool d = MESSAGEQUEUE::destroyLogger();
	bool e = MESSAGEQUEUE::destroyRegister();
	FIFO::remove();

	return (a && b && c && d && e);
}
