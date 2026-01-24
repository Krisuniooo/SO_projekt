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
#define BAKE_MIN_TIME 3.2f
#define BAKE_MAX_TIME 3.3f

// How many products should be added
#define BAKE_MIN_PRODUCTS 65
#define BAKE_MAX_PRODUCTS 120

#define CUSTOMER_MAX_PRODUCT_DEMAND 4
#define CUSTOMER_PRODUCT_BUY_TIME 0.25f

#define CUSTOMER_SPAWN_MIN_TIME 4.25f
#define CUSTOMER_SPAWN_MAX_TIME 7.5f

#define CASHIER_PRODUCT_SCAN_TIME 0.02f

#define PRODUCTS 10
#define MAX_STOCK 256

struct ProductConfig {
	std::string label;
	float price;
	int max_stock;
};

const ProductConfig Products_base[PRODUCTS] = {
	{"WZ-ka", 5.50f, 4},
	{"Kremowka", 8.50f, 5},
	{"Piegusek", 2.50f, 6},
	{"Brownie", 10.25f, 7},
	{"Chocolate Chip", 4.50f, 8},
	{"Coconut cookie", 3.50f, 9},
	{"Chocolate Crinkles", 10.50f, 8},
	{"Amaretti", 6.50f, 12},
	{"Piernik", 6.75f, 4},
	{"Dubai Chocolate", 8.25f, 5},
};

struct Product {
	int unique_id;
	int baked_time;
	int onsale_time;
};

struct Tray {
	Product buffer[MAX_STOCK];
	int head;
	int tail;
	int count;
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
	WZ_SLOTS,
	WZ_COUNT,
	KREMOWKA_MUTEX,
	KREMOWKA_SLOTS,
	KREMOWKA_COUNT,
	PIEGUSEK_MUTEX,
	PIEGUSEK_SLOTS,
	PIEGUSEK_COUNT,
	BROWNIE_MUTEX,
	BROWNIE_SLOTS,
	BROWNIE_COUNT,
	CHOCCHIP_MUTEX,
	CHOCCHIP_SLOTS,
	CHOCCHIP_COUNT,
	COCONUT_MUTEX,
	COCONUT_SLOTS,
	COCONUT_COUNT,
	CHOCCRINKLE_MUTEX,
	CHOCCRINKLE_SLOTS,
	CHOCCRINKLE_COUNT,
	AMARETTI_MUTEX,
	AMARETTI_SLOTS,
	AMARETTI_COUNT,
	PIERNIK_MUTEX,
	PIERNIK_SLOTS,
	PIERNIK_COUNT,
	DUBAI_MUTEX,
	DUBAI_SLOTS,
	DUBAI_COUNT,
	
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
