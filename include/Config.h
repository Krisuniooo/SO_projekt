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

struct SharedData {
	bool is_running; 
	bool is_open;

	int current_customers_count;
	int today_customers_count;
};

enum class SemaphoreTypes {
	TEST,
	TEST2,
	LOGGER_SEM,
	SEM_COUNT
};

struct LogMessage {
	long mtype;
	char text[LOG_MAX_SIZE];
};

/*
inline constexpr Product products[3] = {
	//id, name, price, max_stock
	{0, "Kremowka", 3.50, 10},
	{1, "WZ", 9.99, 5},
	{2, "Piegusek", 1.50, 15}
};
*/

#endif
