#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <vector>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

std::vector<ShoppingList> generateShoppingList() {
	int number_of_products = UTILS::getRandom(2, PRODUCTS);
	
	std::vector<ShoppingList> customer_list;
	std::vector<int> product_ids;
	
	for(int i=0; i<PRODUCTS; i++) {
		product_ids.push_back(i);
	}
	
	for(int i=0; i<number_of_products; i++) {
		int product_id = UTILS::getRandom(0, product_ids.size());
		int product_count = UTILS::getRandom(1, CUSTOMER_MAX_PRODUCT_DEMAND);
		
		ShoppingList item;
		item.id_product = product_ids[product_id];
		item.count = product_count;
		
		customer_list.push_back(item);
		product_ids.erase(product_ids.begin() + product_id);
	}
	
	return customer_list;
}

std::string getClientPIDstring() {
	return std::to_string(getpid());
}


int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	int semid = SEMAPHORE::getID();
	
	std::vector<ShoppingList> shopping_list = generateShoppingList();
	
	if (!SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE))) {
		return 0;
	}
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));

	data->current_customers_count++;
	data->today_customers_count++;
	
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	
	if(!data->is_running || data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - shop closed\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		return 0;
	}
	LOGGER::log("Client " + getClientPIDstring() + " enters shop\n");
	
	for(const auto &i : shopping_list) {
		usleep(CUSTOMER_PRODUCT_BUY_TIME * SIMULATION_MINUTE);
		
		if(data->is_evacuation) {
			LOGGER::log("Client " + getClientPIDstring() + " goes away - evacuation\n");
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
			return 1;
		}
		
		if(SEMAPHORE::lock(data->trays[i.id_product].sem_num)) {
			int take = std::min(i.count, data->trays[i.id_product].in_stock);
			
			data->trays[i.id_product].in_stock -= take;
			LOGGER::log("Client " + getClientPIDstring() + " bought " + std::to_string(take) + " " + Products_base[i.id_product].label + ", wanted " + std::to_string(i.count) +"\n");
		}
		SEMAPHORE::unlock(data->trays[i.id_product].sem_num);
	}
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	data->current_customers_count--;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
	LOGGER::log("Client " + getClientPIDstring() + " goes away - ended shopping\n");
	SHAREDMEMORY::detach();

	return 0;
}
