#ifndef CONFIG_H
#define CONFIG_H

#include "Product.h"
#include <vector>
#include <string>


#define SHARED_MEM_KEY_PATH "/tmp/ciastkarnia_shm"
#define SHARED_MEM_KEY 'M'

// Global product vector
extern std::vector<Product> products;

size_t NUM_PRODUCTS();

#endif
