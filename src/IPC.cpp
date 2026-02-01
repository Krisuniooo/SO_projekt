#include "../include/IPC.h"
#include <iostream>

bool IPC::init() {
	bool shm_initialized = SHAREDMEMORY::init();
	//Log
	if(!shm_initialized) {
		perror("SHARED MEMORY initialization failed");
		IPC::destroyAll();
		return false;
	}
	
	bool sem_initialized = SEMAPHORE::init();
	//Log
	if(!sem_initialized) {
		perror("SEMAPHORE initialization failed");
		IPC::destroyAll();
		return false;
	}
	
	bool mq_initialized = MESSAGEQUEUE::init();
	if(!mq_initialized) {
		perror("MESSEGEQUEUE initialization failed");
		IPC::destroyAll();
		return false;
	}
	
	bool fifo_initialized = FIFO::init();
	if(!fifo_initialized) {
		perror("FIFO initialization failed");
		IPC::destroyAll();
		return false;
	}
	
	return true;
}

bool IPC::destroyAll() {
	bool a = SHAREDMEMORY::detach();
	bool b = SHAREDMEMORY::destroy();
	bool c = SEMAPHORE::destroy();
	bool d = MESSAGEQUEUE::destroyClientMQ();
	bool e = MESSAGEQUEUE::destroyRegister();
	FIFO::remove();

	return (a && b && c && d && e);
}
