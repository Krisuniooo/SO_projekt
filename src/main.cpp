#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <pthread.h>

#include "../include/IPC.h"
#include "../include/Logger.h"
#include "../include/Config.h"



int main() {
    	if (!LOGGER::init()) {
    		std::cerr << "LOGGER initialization error \n";
    		return 1;
    	}

    	if (!IPC::init()) {
    		std::cerr << "IPC initialization error \n";
    		return 1;
    	}
    	
	pthread_t thread_id;
	if (pthread_create(&thread_id, nullptr, LOGGER::logThread, nullptr) != 0) {
		std::cerr << "Thread creation failed";
		return 1;
	}
    	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	if (data == (void*)-1) {
		std::cerr << "Attach failed\n";
		return 1;
	} else {
		std::cout << "Shared memory attached successfully\n";
		LOGGER::log("Shared memory attached successfully\n");
	}

	data->is_running = true;
	data->current_customers_count = 7;
	std::cout << "is_running: " << data->is_running << ",\t customers = " << data->current_customers_count << "\n";

	int semNum = static_cast<int>(SemaphoreTypes::TEST);
    
	std::cout << "New semaphore value: " << SEMAPHORE::getValue(semNum) << "\n";
	LOGGER::log("Changing semaphore value\n");
    
	std::cout << "Locking sem\n";
	if (SEMAPHORE::lock(semNum)) {
		std::cout << "Sem locked\n";
		LOGGER::log("Sem locked\n");
        	sleep(1); 
        
        	std::cout << "Unlocking sem\n";
		if (SEMAPHORE::unlock(semNum)) {
			std::cout << "Sem unlocked\n";
			LOGGER::log("Sem unlocked\n");
		}
	} else {
		std::cerr << "Sem locking error\n";
	}
	LOGGER::endLogThread();
	SHAREDMEMORY::detach();

	return 0;
}
