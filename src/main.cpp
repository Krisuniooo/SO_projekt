#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

#include "../include/IPC.h"
#include "../include/Config.h"


int main() {
    	if (!IPC::init()) {
    		std::cerr << "IPC initialization error \n";
    		return 1;
    	}
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	if (data == (void*)-1) {
		std::cerr << "Attach failed\n";
		return 1;
	} else {
		std::cout << "Shared memory attached successfully\n";
	}

	data->is_running = true;
	data->current_customers_count = 7;
	std::cout << "is_running: " << data->is_running << ",\t customers = " << data->current_customers_count << "\n";

	int semNum = static_cast<int>(SemaphoreTypes::TEST);
    
	SEMAPHORE::setValue(semNum, 1);
	std::cout << "New semaphore value: " << SEMAPHORE::getValue(semNum) << std::endl;
    
	std::cout << "Locking sem\n";
	if (SEMAPHORE::lock(semNum)) {
		std::cout << "Sem locked\n";
        	sleep(1); 
        
        	std::cout << "Unlocking sem\n";
		if (SEMAPHORE::unlock(semNum)) {
			std::cout << "Sem unlocked\n";
		}
	} else {
		std::cerr << "Sem locking error\n";
	}

	SHAREDMEMORY::detach();

	return 0;
}
