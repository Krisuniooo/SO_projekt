#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"

void generateBaker() {
	pid_t pid = fork();
	if(pid == -1) {
    		std::cerr << "IPC initialization error \n";
    		exit(1);
    	} else if(pid == 0) {
    		std::cout << "Generating Baker\n";
    		execl("./baker", "baker", NULL);
    		std::cerr << "Baker process could not be created! \n";
    		exit(1);
    	}
}

int main() {
    	if (!LOGGER::init()) {
    		std::cerr << "LOGGER initialization error \n";
    		return 1;
    	}

    	if (!IPC::init()) {
    		std::cerr << "IPC initialization error \n";
    		return 1;
    	}
    	SEMAPHORE::setValue(static_cast<int>(SemaphoreTypes::LOGGER_SEM_MUTEX), 0);
    	
	pthread_t thread_id;
	if (pthread_create(&thread_id, nullptr, LOGGER::logThread, nullptr) != 0) {
		std::cerr << "Thread creation failed";
		return 1;
	}
	
	generateBaker();
    	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	if (data == (void*)-1) {
		std::cerr << "Attach failed\n";
		return 1;
	} else {
		std::cout << "Shared memory attached successfully\n";
		LOGGER::log("Shared memory attached successfully\n");
	}
	
	LOGGER::endLogThread();

	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::LOGGER_SEM_MUTEX));
	
	bool destroy_success = IPC::destroyAll();
	if(!destroy_success) {
		std::cerr << "could not destroy IPC\n";
	}

	return 0;
}
