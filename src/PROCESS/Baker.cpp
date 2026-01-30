#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"
#include "../../include/IPC/Fifo.h"

#include <cstdio>
#include <cmath>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <vector>
#include <pthread.h>
#include <ctime>

static pthread_t trays_thread_ids[PRODUCTS];
static SharedData* data;
static int baked_product_id = 0;
static bool sigterm_handle = false;


void* bakeProductOnTray(void* arg) {
	int id_product = *static_cast<int*>(arg);
	delete static_cast<int*>(arg);

	while(data->is_running) {
		Product newProduct;
		std::time_t time_now;
		time(&time_now);
		newProduct.baked_time = time_now;
		int product_count = UTILS::getRandom(BAKE_MIN_PRODUCTS, BAKE_MAX_PRODUCTS);
		
		if(!data->is_running || data->is_evacuation) break;
	
		for(int i=0; i<product_count; i++) {
			if(SEMAPHORE::lock(UTILS::SEM_INDEX_SLOTS(id_product))) {
				
				if(!data->is_running || data->is_evacuation) {
					SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
					break;
				}
			
				if(SEMAPHORE::lock(UTILS::SEM_INDEX_MUTEX(id_product))) {
				
					if(!data->is_running || data->is_evacuation) {
						SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
						break;
					}
					
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
					
						if(!data->is_running || data->is_evacuation) {
							SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
							SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(id_product));
							SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
							break;
						}
					
						newProduct.unique_id = baked_product_id++;
						time(&time_now);
						newProduct.onsale_time = time_now;
					
						data->trays[id_product].buffer[data->trays[id_product].tail] = newProduct;
						data->trays[id_product].tail = (data->trays[id_product].tail + 1) % Products_base[id_product].max_stock;
						data->trays[id_product].count++;
						data->total_produced[id_product]++;
					
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						//SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_COUNT(id_product));
						
						std::cout << "Added " << Products_base[id_product].label << " new value: " << SEMAPHORE::getValue(UTILS::SEM_INDEX_COUNT(id_product)) << "\n";
						
					} else {
						SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
					}	
				} else {
					SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(id_product));
				}
			}
		}
		
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME);
		for (int i = 0; i < 100; ++i) {
    			if (!data->is_running || data->is_evacuation) break;
			usleep((time/100.0) * SIMULATION_MINUTE);
		}
	}
	
	return nullptr;
}

void handlerSigTerm(int sig) {
	sigterm_handle = true;
}

int main() {
	signal(SIGTERM, handlerSigTerm);
	
	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();

	for(int i=0; i< PRODUCTS; i++) {
		int* id = new int(i);
		pthread_t tid;
		pthread_create(&tid, NULL, bakeProductOnTray, id);
		trays_thread_ids[i] = tid;
	}

	while(data->is_running) {
		sleep(1);	
		if(data->is_evacuation)
			break;
		
		if(sigterm_handle) 
			break;
	}
	
	for(int i=0; i< PRODUCTS; i++) {
		pthread_join(trays_thread_ids[i], nullptr);
		trays_thread_ids[i] = -1;
	}
	
	SHAREDMEMORY::detach();
	
	return 0;
}
