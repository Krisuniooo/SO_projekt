#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

class Product {
	private:
		unsigned int index;
		std::string name;
		const float price;
		unsigned int max_stock;

	public:
		Product(const unsigned int _index, const std::string _name, const float _price, const unsigned int _max_stock) : index(_index), name(_name), price(_price), max_stock(_max_stock) {};


		// Getters
		int get_id() const;
		int get_max_stock() const;
		float get_price() const;


		// Setters
		// ...

		// Operations
		void print() const;
};

#endif
