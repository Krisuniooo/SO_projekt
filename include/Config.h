#ifndef CONFIG_H
#define CONFIG_H

#include "Product.h"
#include <vector>
#include <string>

// Global product vector
extern std::vector<Product> products;

namespace CONFIG {
	size_t NUM_PRODUCTS();

	constexpr unsigned int RANDOM_PRODUCTS_BAKED_MIN = 2;
	constexpr unsigned int RANDOM_PRODUCTS_BAKED_MAX = 10;

	const std::string SHARED_MEM_KEY_PATH = "/tmp/ciastkarnia_shm";
	const std::string LOG_FILE_PATH = "data/log.txt";
}

#endif
