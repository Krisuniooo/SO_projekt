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

static int trays_write_fd[PRODUCTS];
static pthread_t trays_thread_ids[PRODUCTS];
SharedData* data;
static int id = 0;

void* bakeProductOnTray(void* arg) {
	int id_product = *static_cast<int*>(arg);
	delete static_cast<int*>(arg);

	while(true) {
	
		Product newProduct;
		std::time_t time_now;
		time(&time_now);
		newProduct.baked_time = time_now;
		int product_count = UTILS::getRandom(BAKE_MIN_PRODUCTS, BAKE_MAX_PRODUCTS);
	
		for(int i=0; i<product_count; i++) {
			if(SEMAPHORE::lock(UTILS::SEM_INDEX_SLOTS(id_product))) {
				if(SEMAPHORE::lock(UTILS::SEM_INDEX_MUTEX(id_product))) {
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {					newProduct.unique_id = id++;
						time(&time_now);
						newProduct.onsale_time = time_now;
					
						data->trays[id_product].buffer[data->trays[id_product].tail] = newProduct;
						data->trays[id_product].tail = (data->trays[id_product].tail + 1) % Products_base[id_product].max_stock;
						data->trays[id_product].count++;
					
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
		
		//int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME);
		//sleep(static_cast<int>(time));
	}
	
	
	
	return nullptr;
}

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();

	for(int i=0; i< PRODUCTS; i++) {
		int* id = new int(i);
		pthread_t tid;
		pthread_create(&tid, NULL, bakeProductOnTray, id);
		trays_thread_ids[i] = tid;
	}

	while(true) {
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		usleep(static_cast<int>(time/10));
		
		if(data->is_evacuation)
			break;
	}
	
	
	SHAREDMEMORY::detach();
	for(int i=0; i<PRODUCTS; i++) {
		close(trays_write_fd[i]);
	}
	
	return 0;
}
