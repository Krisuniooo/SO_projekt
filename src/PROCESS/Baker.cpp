#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}


int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();

	while(true) {
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		//usleep(static_cast<int>(1000));
		
		if(data->is_evacuation)
			break;
			
		for(int i = 0; i<PRODUCTS; i++) {
			int amount = UTILS::getRandom(BAKE_MIN_PRODUCTS, BAKE_MAX_PRODUCTS);
			
			std::string log_message;
			if(SEMAPHORE::lock(data->trays[i].sem_num)) {
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
				
					if((data->trays[i].in_stock + amount) > Products_base[i].max_stock) {
						//log_message = "Baker: added " + std::to_string(Products_base[i].max_stock - data->trays[i].in_stock) + ", trashed " + std::to_string(data->trays[i].in_stock + amount - Products_base[i].max_stock) +  " " + Products_base[i].label + ", new value: " + std::to_string(data->trays[i].in_stock + amount) + "\n";
						
						#if DEBUG_MESSAGES == 1
							std::cout << log_message;
						#endif
										
						data->trays[i].in_stock = Products_base[i].max_stock;
						std::cout << data->trays[i].in_stock << "\n";
					} else {
						//log_message = "Baker: added " + std::to_string(amount) + " " + Products_base[i].label + ", new value: " + std::to_string(data->trays[i].in_stock + amount) + "\n";
						
						#if DEBUG_MESSAGES == 1
							std::cout << log_message;
						#endif
						
						
						data->trays[i].in_stock = data->trays[i].in_stock + amount;
					}
					
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					SEMAPHORE::unlock(data->trays[i].sem_num);
					//LOGGER::log(log_message);
				}
			}
		}
	}
	
	SHAREDMEMORY::detach();
	
	return 0;
}
