#include "../include/Product.h"
#include "../include/IPC/SharedMemory.h"
#include "../include/Logger.h"

// Getters
int Product::get_id() const			{ return this->index; }
int Product::get_max_stock() const	 	{ return this->max_stock; }
float Product::get_price() const		{ return this->price; }

// Setters
// ...

//Operations
