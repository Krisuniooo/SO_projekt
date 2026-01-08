#include "../include/Config.h"

std::vector<Product> products = {
	//id, name, price, max_stock
	{0, "Kremowka", 3.50, 10},
	{1, "WZ", 9.99, 5},
	{2, "Piegusek", 1.50, 15}
};

size_t NUM_PRODUCTS() {
	return products.size();
}
