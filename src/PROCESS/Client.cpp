#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <vector>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/MessageQueue.h"
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
		int product_id = UTILS::getRandom(0, product_ids.size()-1);
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
	int mq_register_id = MESSAGEQUEUE::getRegisterID();
	
	std::vector<ShoppingList> shopping_list = generateShoppingList();
	
	if (!SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE))) {
		return 0;
	}
	
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {

		data->current_customers_count++;
		data->today_customers_count++;
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	} else {
		return 0;
	}
	
	if(!data->is_running || data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - shop closed\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		return 0;
	}
	LOGGER::log("Client " + getClientPIDstring() + " enters shop\n");
	
	int index = 0;
	for(auto &i : shopping_list) {
		if(data->is_evacuation) {
			LOGGER::log("Client " + getClientPIDstring() + " goes away - evacuation\n");
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
			return 1;
		}
		//usleep(CUSTOMER_PRODUCT_BUY_TIME * SIMULATION_MINUTE);
		
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) + i.id_product + 1)) {
			int take = std::min(i.count, data->trays[i.id_product].in_stock);
			
			data->trays[i.id_product].in_stock -= take;
			LOGGER::log("Client " + getClientPIDstring() + " took " + std::to_string(take) + " " + Products_base[i.id_product].label + ", wanted " + std::to_string(i.count) +"\n");
			i.count = take;
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) + i.id_product + 1);
		}
		
		index++;
	}
	
	if (!data->is_evacuation) {
		ReceiptMessage msg;
		msg.mtype = 1;
		msg.client = getpid();

		bool create_receipt = false;
		for(int i=0; i<PRODUCTS; i++) {
			msg.counts[i] = 0;
			for(auto &item : shopping_list) {
				if(item.id_product == i) {
					msg.counts[i] = item.count;
					if(item.count > 0) create_receipt = true;
				}
			}
		}

		if(create_receipt) {
			SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			bool second_register_active = data->second_register_active;
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			
			if(second_register_active) {
				msg.mtype = UTILS::getRandom(1, 2);
			}
			
			LOGGER::log("Client " + getClientPIDstring() + " gives product list to " + std::to_string(msg.mtype) + "\n");
			if(msgsnd(mq_register_id, &msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
				std::cerr << "could not send shopping list to cashier\n";
			}
			
			
			ReceiptMessage msg_rcv;
			std::cout << "\t\t" << getpid() << "\n";
			while(msgrcv(mq_register_id, &msg_rcv, sizeof(ReceiptMessage) - sizeof(long), getpid(), IPC_NOWAIT) == -1) {
				usleep(10000);
			}
			LOGGER::log("Client " + getClientPIDstring() + " recieves checkout\n");
		}
	}
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	data->current_customers_count--;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
	LOGGER::log("Client " + getClientPIDstring() + " goes away - ended shopping\n");
	SHAREDMEMORY::detach();

	return 0;
}
