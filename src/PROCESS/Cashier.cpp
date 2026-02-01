#include "../../include/Config.h"
#include "../../include/Utils.h"
#include "../../include/Logger.h"
#include "../../include/Signals.h"
#include "../../include/IPC/MessageQueue.h"
#include "../../include/IPC/SharedMemory.h"
#include "../../include/IPC/Semaphore.h"

int main(int argc, char *argv[]) {
	int cashier_id = std::stoi(argv[1]);
	
	SharedData* data = static_cast<SharedData*>(SHAREDMEMORY::attach());
	SEMAPHORE::getID();
	int mq_register_id = MESSAGEQUEUE::getRegisterID();
	int mq_client_id = MESSAGEQUEUE::getClientMQID();
	
	ReceiptMessage my_msg;
	
	if(!UTILS::setupKeyFile(RECEIPT_PATH)) {
		perror("Could not create Receipt file");
		return 1;
	}
	
	FILE* file = fopen(RECEIPT_PATH, "w");
	if(!file) {
		perror("Could not open Receipt file");
		return 1;
	}
	
	LOGGER::log("Register " + std::to_string(cashier_id) + " process starts\n");
	
	while(data->is_running) {
		
		if(msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, 0) == -1) {
			if (errno == EINTR) {
				break;
			}
		
			if(data->is_evacuation) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " is forcefullyy closing\n");
				if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
					printf("FORCEFULLY CLOSING REGISTER %d\n", cashier_id);
					SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
				}
				
				break;
			}
		} else {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
			
			usleep(350000);
			
			LOGGER::log("Register " + std::to_string(cashier_id) + " starts scanning products " + std::to_string(my_msg.client) + "\n");
			
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX))) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " locks mutex for " + std::to_string(my_msg.client) + "\n");
			
				float total = 0;
				
				fprintf(file, "\n==================================\n");
				fprintf(file, "	RECEIPT - CLIENT %d", my_msg.client);
				fprintf(file, "\n==================================\n");
				
				for(int i = 0; i < PRODUCTS; i++) {
					if(my_msg.counts[i] > 0) {
					
						SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						data->total_sold[i] += my_msg.counts[i];
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					
						float total_local = my_msg.counts[i] * Products_base[i].price;
						fprintf(file, "%s %d - %.2f\n", (Products_base[i].label).c_str(), my_msg.counts[i], total_local);
						total += total_local;
					}
				}
				fprintf(file, "==================================\n");
				fprintf(file, "TOTAL: %.2f$", total);
				fprintf(file, "\n==================================\n");
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
				
				LOGGER::log("Register sends checkout to " + std::to_string(my_msg.mtype) + "\n");
				
				my_msg.mtype = my_msg.client;
				if(msgsnd(mq_client_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0) == -1) {
					if(errno == EINTR) {
						break;
					}
					perror("could not send shopping list to client");
				}
				
				if(data->is_evacuation) {
					LOGGER::log("Register " + std::to_string(cashier_id) + " is preparing to close\n");
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
						printf("PREPARING TO CLOSE REGISTER %d\n", cashier_id);
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
					}
					break;
				}
			}
			LOGGER::log("Register " + std::to_string(cashier_id) + " finished scanning products " + std::to_string(my_msg.client) + "\n");
		}
	}
	
	if(!data->is_evacuation) {
		while (msgrcv(mq_register_id, &my_msg, sizeof(my_msg) - sizeof(long), cashier_id, IPC_NOWAIT) != -1) {
			LOGGER::log("Register " + std::to_string(cashier_id) + " serves the customer " + std::to_string(my_msg.client) + "\n");
			
			usleep(350000);
			
			LOGGER::log("Register " + std::to_string(cashier_id) + " starts scanning products " + std::to_string(my_msg.client) + "\n");
			
			if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX))) {
				LOGGER::log("Register " + std::to_string(cashier_id) + " locks mutex for " + std::to_string(my_msg.client) + "\n");
			
				float total = 0;
				
				fprintf(file, "\n==================================\n");
				fprintf(file, "	RECEIPT - CLIENT %d", my_msg.client);
				fprintf(file, "\n==================================\n");
				
				for(int i = 0; i < PRODUCTS; i++) {
					if(my_msg.counts[i] > 0) {
					
						SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
						data->total_sold[i] += my_msg.counts[i];
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::SHARED_DATA_MUTEX));
					
						float total_local = my_msg.counts[i] * Products_base[i].price;
						fprintf(file, "%s %d - %.2f\n", (Products_base[i].label).c_str(), my_msg.counts[i], total_local);
						total += total_local;
					}
				}
				fprintf(file, "==================================\n");
				fprintf(file, "TOTAL: %.2f$", total);
				fprintf(file, "\n==================================\n");
				
				SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::RECEIPT_MUTEX));
				
				LOGGER::log("Register sends checkout to " + std::to_string(my_msg.mtype) + "\n");
				
				my_msg.mtype = my_msg.client;
				if(msgsnd(mq_client_id, &my_msg, sizeof(ReceiptMessage) - sizeof(long), 0) == -1) {
					if(errno == EINTR) {
						break;
					}
					perror("could not send shopping list to client");
				}

							
				if(data->is_evacuation) {
					LOGGER::log("Register " + std::to_string(cashier_id) + " is preparing to close\n");
					if(SEMAPHORE::lock(static_cast<int>(SemaphoreTypes::COUT_MUTEX))) {
						printf("PREPARING TO CLOSE REGISTER %d\n", cashier_id);
						SEMAPHORE::unlock(static_cast<int>(SemaphoreTypes::COUT_MUTEX));
					}
					break;
				}
			}
			LOGGER::log("Register " + std::to_string(cashier_id) + " finished scanning products " + std::to_string(my_msg.client) + "\n");
		}
	}
	
	LOGGER::log("Register " + std::to_string(cashier_id) + " finished job\n");
	
	fclose(file);
	SHAREDMEMORY::detach();
	return 0;
}
