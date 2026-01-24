#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <limits.h>


#define SHARED_MEM_KEY_PATH "/tmp/ciastkarnia_shm"
#define SHARED_MEM_KEY 'M'

#define SEMAPHORE_KEY_PATH "/tmp/ciastkarnia_sem"
#define SEMAPHORE_KEY 'S'

#define MESSAGE_QUEUE_KEY_PATH "/tmp/ciastkarnia_mq"
#define MESSAGE_QUEUE_KEY 'Q'
#define MESSAGE_QUEUE_REGISTER_KEY 'R'

#define FIFO_PATH "/tmp/ciastkarnia_fifo_"

#define LOGGER_PATH "data/data.log"
#define LOG_MAX_SIZE 512

#define RECEIPT_PATH "data/receipt.log"
#define LOG_MAX_LINE_SIZE 512

#define DEBUG_MESSAGES 0

#define MAX_CLIENT_INSIDE 10 				// N
#define OPENING_HOUR 8 				// Tp
#define CLOSING_HOUR 20 				// Tk

#define SIMULATION_MINUTE 1000000			// simulation minute in usleep
#define SIMULATION_HOUR (60 * SIMULATION_MINUTE)	// simulation hour in usleep

// Time for baking products
#define BAKE_MIN_TIME 3.0f
#define BAKE_MAX_TIME 5.0f

// How many products should be added
#define BAKE_MIN_PRODUCTS 15
#define BAKE_MAX_PRODUCTS 20

#define CUSTOMER_MAX_PRODUCT_DEMAND 4
#define CUSTOMER_PRODUCT_BUY_TIME 0.25f

#define CUSTOMER_SPAWN_MIN_TIME 1.25f
#define CUSTOMER_SPAWN_MAX_TIME 1.5f

#define CASHIER_PRODUCT_SCAN_TIME 0.02f

#define PRODUCTS 10

struct ProductConfig {
	std::string label;
	float price;
	int max_stock;
};

const ProductConfig Products_base[PRODUCTS] = {
	{"WZ-ka", 5.50f, 4},
	{"Kremowka", 8.50f, 64},
	{"Piegusek", 2.50f, 128},
	{"Brownie", 10.25f, 32},
	{"Chocolate Chip", 4.50f, 128},
	{"Coconut cookie", 3.50f, 32},
	{"Chocolate Crinkles", 10.50f, 8},
	{"Amaretti", 6.50f, 8},
	{"Piernik", 6.75f, 4},
	{"Dubai Chocolate", 8.25f, 16},
};

struct Tray {
	int id_product;
	
	int in_stock;
	int sem_num;
	
	int produced_total;
	int sold_total;
};

struct SharedData {
	bool is_running; 
	bool is_open;
	
	bool is_stocktaking; 	// signal1
	bool is_evacuation; 	// signal2 

	int current_customers_count;
	int today_customers_count;
	
	Tray trays[PRODUCTS];
	
	bool second_register_active;
};

enum class SemaphoreTypes {
	SHARED_DATA_MUTEX,
	CLIENTS_INSIDE,
	RECEIPT_MUTEX,
	
	PRODUCTS_BASE, // do not remove used to track tray ids
	WZ_MUTEX,
	KREMOWKA_MUTEX,
	PIEGUSEK_MUTEX,
	BROWNIE_MUTEX,
	CHOCCHIP_MUTEX,
	COCONUT_MUTEX,
	CHOCCRINKLE_MUTEX,
	AMARETTI_MUTEX,
	PIERNIK_MUTEX,
	DUBAI_MUTEX,
	PRODUCTS_BASE_END, // do not remove used to track tray ids
	
	SEM_COUNT
};

struct SemaphoreInit {
    SemaphoreTypes type;
    int value;
};

const SemaphoreInit SemConfig[] = {
	{ SemaphoreTypes::SHARED_DATA_MUTEX, 1 },
	{ SemaphoreTypes::CLIENTS_INSIDE, MAX_CLIENT_INSIDE },
	{ SemaphoreTypes::RECEIPT_MUTEX, 1 },

	{ SemaphoreTypes::PRODUCTS_BASE, 1 },
	{ SemaphoreTypes::WZ_MUTEX, 1 },
	{ SemaphoreTypes::KREMOWKA_MUTEX, 1 },
	{ SemaphoreTypes::PIEGUSEK_MUTEX, 1 },
	{ SemaphoreTypes::BROWNIE_MUTEX, 1 },
	{ SemaphoreTypes::CHOCCHIP_MUTEX, 1 },
	{ SemaphoreTypes::COCONUT_MUTEX, 1 },
	{ SemaphoreTypes::CHOCCRINKLE_MUTEX, 1 },
	{ SemaphoreTypes::AMARETTI_MUTEX, 1 },
	{ SemaphoreTypes::PIERNIK_MUTEX, 1 },
	{ SemaphoreTypes::DUBAI_MUTEX, 1 },
	{ SemaphoreTypes::PRODUCTS_BASE_END, 1 },
	{ SemaphoreTypes::SEM_COUNT, 1 }
};

struct LogMessage {
	long mtype;
	char text[LOG_MAX_SIZE];
};

struct ReceiptMessage {
	long mtype;
	pid_t client;
	int counts[PRODUCTS];
};

struct ShoppingList {
	int id_product;
	int count;
};

#endif
