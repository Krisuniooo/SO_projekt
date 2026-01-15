#ifndef CONFIG_H
#define CONFIG_H

#include "Product.h"
#include <string>


#define SHARED_MEM_KEY_PATH "/tmp/ciastkarnia_shm"
#define SHARED_MEM_KEY 'M'

#define SEMAPHORE_KEY_PATH "/tmp/ciastkarnia_sem"
#define SEMAPHORE_KEY 'S'

#define MESSAGE_QUEUE_KEY_PATH "/tmp/ciastkarnia_mq"
#define MESSAGE_QUEUE_KEY 'Q'

#define LOGGER_PATH "data/data.log"
#define LOG_MAX_SIZE 255

#define DEBUG_MESSAGES 0

#define MAX_CLIENT_INSIDE 10 				// N
#define MAX_CASH_REGISTERS (MAX_CLIENT_INSIDE / 2)	//K
#define OPENING_HOUR 8 				// Tp
#define CLOSING_HOUR 20 				// Tk

#define SIMULATION_MINUTE 1000000			// simulation minute in usleep
#define SIMULATION_HOUR (60 * SIMULATION_MINUTE)	// simulation hour in usleep

// Time for baking products
#define BAKE_MIN_TIME 1.0f
#define BAKE_MAX_TIME 3.0f

// How many products should be added
#define BAKE_MIN_PRODUCTS 2
#define BAKE_MAX_PRODUCTS 6

#define PRODUCTS 3

struct ProductConfig {
	std::string label;
	float price;
	int max_stock;
};

const ProductConfig Products_base[PRODUCTS] = {
	{"WZ-ka", 5.50f, 10},
	{"Kremowka", 8.50f, 15},
	{"Piegusek", 2.50f, 20}
};

struct Tray {
	int id_product;
	std::string label;
	float price;
	
	int max_stock;
	int in_stock;
	int sem_id;
	
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
};

enum class SemaphoreTypes {
	SHARED_DATA_MUTEX,
	CLIENTS_INSIDE_LIMIT,
	
	PRODUCTS_BASE, // do not remove used to track tray ids
	WZ_MUTEX,
	KREMOWKA_MUTEX,
	PIEGUSEK_MUTEX,
	PRODUCTS_BASE_END, // do not remove used to track tray ids
	
	SEM_COUNT
};

struct LogMessage {
	long mtype;
	char text[LOG_MAX_SIZE];
};

#endif
