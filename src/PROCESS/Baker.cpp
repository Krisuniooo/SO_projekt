#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"
#include "../../include/IPC/Fifo.h"

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
						
						LOGGER::log("Baker added " + Products_base[id_product].label + " to the tray\n");
						if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
							printf(BAKER_COLOR"Added %s"RESET_COLOR", new value: %d\n", (Products_base[id_product].label).c_str(), SEMAPHORE::getValue(UTILS::SEM_INDEX_COUNT(id_product)));
							SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
						}
						
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
		usleep(time * SIMULATION_MINUTE);
	}
	
	return nullptr;
}

void handlerSigTerm(int sig) {
	sigterm_handle = true;
}

int main() {
	signal(SIGTERM, handlerSigTerm);
	
	sigset_t mask, oldmask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &mask, &oldmask);
	
	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	SEMAPHORE::getID();

	for(int i=0; i< PRODUCTS; i++) {
		int* id = new int(i);
		pthread_t tid;
		pthread_create(&tid, NULL, bakeProductOnTray, id);
		trays_thread_ids[i] = tid;
	}
	
	while(data->is_running && !data->is_evacuation && !sigterm_handle) {
		sigsuspend(&oldmask);
	}
	
	for(int i=0; i< PRODUCTS; i++) {
		pthread_join(trays_thread_ids[i], nullptr);
		trays_thread_ids[i] = -1;
	}
	
	SHAREDMEMORY::detach();
	
	return 0;
}
