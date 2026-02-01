#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/Signals.h"
#include "../../include/IPC/MessageQueue.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"
#include "../../include/IPC/Fifo.h"

static int mq_register_id = -1;
static int mq_client_id = -1;
static SharedData* data;

static void handleSigKill(int sig) {
	SIGNALS::send(getpid(), SIGRTMIN + 1);
	
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

void handleReceipt(std::map<int, int> shopping_list) {
	if(mq_register_id == -1) return;

	ReceiptMessage msg;
	msg.mtype = 1;
	msg.client = getpid();

	bool create_receipt = false;
	
	for(int i=0; i<PRODUCTS; i++) {
		msg.counts[i] = 0;
	}
	
	for(auto i : shopping_list) {
		msg.counts[i.first] = i.second;
		create_receipt = true;
	}
	
	if(!create_receipt) return;
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			
	if(data->second_register_active) {
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		msg.mtype = UTILS::getRandom(1, 2);
		
		LOGGER::log("Client " + getClientPIDstring() + " gives product list to " + std::to_string(msg.mtype) + "\n");
		if(msgsnd(mq_register_id, &msg, sizeof(ReceiptMessage) - sizeof(long), 0) == -1) {
			perror("could not send shopping list to cashier");
			return;
		}
	} else {
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		
		LOGGER::log("Client " + getClientPIDstring() + " gives product list to " + std::to_string(msg.mtype) + "\n");
		if(msgsnd(mq_register_id, &msg, sizeof(ReceiptMessage) - sizeof(long), 0) == -1) {
			perror("could not send shopping list to cashier");
			return;
		}
	}
	if(data->is_evacuation) {
		return;
	}
			
	LOGGER::log("Client " + getClientPIDstring() + " is waiting for checkout\n");
	ReceiptMessage msg_rcv;
	while(msgrcv(mq_client_id, &msg_rcv, sizeof(ReceiptMessage) - sizeof(long), getpid(), 0) == -1) {
		if(data->is_evacuation) {
			break;
		}
		if(errno == EINTR) {
			printf("Could not recieve checkout\n");
			break;
		}
	
		continue;
	}
	LOGGER::log("Client " + getClientPIDstring() + " recieves checkout\n");
}


int main() {
	SIGNALS::init(SIGRTMIN + 1);
	signal(SIGINT, handleSigKill);
	signal(SIGTERM, handleSigKill);
	
	
	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	SEMAPHORE::getID();
	mq_register_id = MESSAGEQUEUE::getRegisterID();
	mq_client_id = MESSAGEQUEUE::getClientMQID();
	
	std::vector<ShoppingList> shopping_list_demand = generateShoppingList();
	
	if(!data->is_open || data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - shop closed\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
		return 0;
	}
	
	if (!SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE))) {
		LOGGER::log("Client " + getClientPIDstring() + " could not enter shop - semaphore error\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
		return 0;
	}
	
	if(!data->is_open || data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - shop closed\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
		return 0;
	}
	
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {

		data->current_customers_count++;
		data->today_customers_count++;
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	} else {
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		LOGGER::log("Client " + getClientPIDstring() + " could not enter shop - semaphore error\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
		return 0;
	}
	
	if(data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - evacuation ongoing\n");
		if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
			data->current_customers_count--;
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		}
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
		return 0;
	}
	LOGGER::log("Client " + getClientPIDstring() + " enters shop\n");
	
	std::map<int, int> shopping_list_acquired;
	for(auto &i : shopping_list_demand) {
		if(data->is_evacuation) {
			LOGGER::log("Client " + getClientPIDstring() + " goes away - evacuation\n");
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
				data->current_customers_count--;
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			}
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
			return 1;
		}
		
		//usleep(CUSTOMER_PRODUCT_BUY_TIME * SIMULATION_MINUTE);
		
		for(int j = 0; j<i.count; j++) {
			LOGGER::log("Client " + getClientPIDstring() + " tries to take product " + Products_base[i.id_product].label + " if possible\n");
			if(SEMAPHORE::lock(UTILS::SEM_INDEX_COUNT(i.id_product), true)) {
				LOGGER::log("Client " + getClientPIDstring() + " is locking product " + Products_base[i.id_product].label + " mutex\n");
				if(SEMAPHORE::lock(UTILS::SEM_INDEX_MUTEX(i.id_product))) {
					LOGGER::log("Client " + getClientPIDstring() + " is locking shared data mutex\n");
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {
						printf("Client %d took %s\n", getpid(), (Products_base[i.id_product].label).c_str());
					
						data->trays[i.id_product].head = (data->trays[i.id_product].head + 1) % Products_base[i.id_product].max_stock;
						data->trays[i.id_product].count--;
					
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(i.id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_SLOTS(i.id_product));
						
						shopping_list_acquired[i.id_product]++;
					} else {
						SEMAPHORE::unlock(UTILS::SEM_INDEX_COUNT(i.id_product));
						SEMAPHORE::unlock(UTILS::SEM_INDEX_MUTEX(i.id_product));
					}
				} else {
					SEMAPHORE::unlock(UTILS::SEM_INDEX_COUNT(i.id_product));
					break;
				}
			} else {
				LOGGER::log("Client " + getClientPIDstring() + " could not take " + Products_base[i.id_product].label + " goes to next\n");
				break;
			}
		}
	}
	
	if (!data->is_evacuation) {
		//TO CHANGE
		handleReceipt(shopping_list_acquired);
	}
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	data->current_customers_count--;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
	LOGGER::log("Client " + getClientPIDstring() + " goes away - ended shopping\n");
	if(data->current_customers_count == 0) SIGNALS::send(getppid(), SIGRTMIN + 2);
	SHAREDMEMORY::detach();

	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::PROCESSES_MAX));
	return 0;
}
