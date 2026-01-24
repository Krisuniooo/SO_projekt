#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/IPC/MessageQueue.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"
#include "../../include/IPC/Fifo.h"

#include <sys/ioctl.h>
#include <poll.h>

static int mq_register_id = -1;
static SharedData* data;

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

void handleReceipt(std::vector<ShoppingList> shopping_list) {
	if(mq_register_id == -1) return;

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
	
	if(!create_receipt) return;
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	bool second_register_active = data->second_register_active;
	
			
	if(second_register_active) {
		msg.mtype = UTILS::getRandom(1, 2);
		
		LOGGER::log("Client " + getClientPIDstring() + " gives product list to " + std::to_string(msg.mtype) + "\n");
		if(msgsnd(mq_register_id, &msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
			std::cerr << "could not send shopping list to cashier\n";
		}
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	} else {
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
		
		LOGGER::log("Client " + getClientPIDstring() + " gives product list to " + std::to_string(msg.mtype) + "\n");
		if(msgsnd(mq_register_id, &msg, sizeof(ReceiptMessage) - sizeof(long), 0)) {
			std::cerr << "could not send shopping list to cashier\n";
		}
	}
			
			
	ReceiptMessage msg_rcv;
	while(msgrcv(mq_register_id, &msg_rcv, sizeof(ReceiptMessage) - sizeof(long), getpid(), 0) == -1) {
		continue;
	}
	LOGGER::log("Client " + getClientPIDstring() + " recieves checkout\n");
}


int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	SEMAPHORE::getID();
	mq_register_id = MESSAGEQUEUE::getRegisterID();
	
	std::vector<ShoppingList> shopping_list = generateShoppingList();
	
	if (!SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE))) {
		return 0;
	}
	
	if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX))) {

		data->current_customers_count++;
		data->today_customers_count++;
	
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	} else {
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		return 0;
	}
	
	if(!data->is_running || data->is_evacuation) {
		LOGGER::log("Client " + getClientPIDstring() + " goes away - shop closed\n");
		SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
		return 0;
	}
	LOGGER::log("Client " + getClientPIDstring() + " enters shop\n");
	
	
	for(auto &i : shopping_list) {
		if(data->is_evacuation) {
			LOGGER::log("Client " + getClientPIDstring() + " goes away - evacuation\n");
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
			return 1;
		}
		
		//usleep(CUSTOMER_PRODUCT_BUY_TIME * SIMULATION_MINUTE);
		
		
		int product_size_bytes = PIPE_BUF / Products_base[i.id_product].max_stock;
		
		std::string path = FIFO_PATH + std::to_string(i.id_product);
		int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
		if(fd == -1) { 
			SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			data->current_customers_count--;
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
			SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
			LOGGER::log("Client " + getClientPIDstring() + " goes away - ended shopping\n");
			SHAREDMEMORY::detach();
			
			return -1;
		}
		
		char bytedata[product_size_bytes];
		
		int bytes = 0;
		for(int j=0; j< i.count; j++) {
			//int bytes_available;
			//if (ioctl(fd, FIONREAD, &bytes_available) == -1) {
			//    perror("Błąd ioctl");
			//    break;
			//}
			//if(bytes_available >= product_size_bytes) {
				int newbytes = read(fd, bytedata, product_size_bytes);
				
				if (newbytes > 0) {
					bytes += newbytes;
					printf("Odebrano: %d\n", (int)bytes);
				} else if (bytes == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
					printf("Brak danych, sprawdzam ponownie za sekundę...\n");
					break;
				} else if (bytes == 0) {
					printf("Pisarz zakończył pracę.\n");
					break;
				}
			//}
		}
		close(fd);
		
		if(bytes > 0)
			LOGGER::log("Client " + getClientPIDstring() + " took " + std::to_string(bytes) + " bytes of " + Products_base[i.id_product].label + "\n");
	}
	
	if (!data->is_evacuation) {
		handleReceipt(shopping_list);
	}
	
	SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	data->current_customers_count--;
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
	SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::CLIENTS_INSIDE));
	LOGGER::log("Client " + getClientPIDstring() + " goes away - ended shopping\n");
	SHAREDMEMORY::detach();

	return 0;
}
