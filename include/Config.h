#ifndef CONFIG_H
#define CONFIG_H

#include "Product.h"
#include <vector>
#include <string>


#define SHARED_MEM_KEY_PATH "/tmp/ciastkarnia_shm"
#define SHARED_MEM_KEY 'M'

#define SEMAPHORE_KEY_PATH "/tmp/ciastkarnia_sem"
#define SEMAPHORE_KEY 'S'

#define LOGGER_PATH "../data/data.log"

enum class SemaphoreTypes {
	TEST,
	TEST2,
	SEM_COUNT
};

// Global product vector
extern std::vector<Product> products;

size_t NUM_PRODUCTS();

#endif
