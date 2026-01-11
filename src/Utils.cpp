#include "../include/Utils.h"

#include <fstream>
#include <string>

bool UTILS::setupKeyFile(const char* path) {
	std::ofstream file(path, std::ios::app);
	
	if(!file.is_open()) 
		return false;
		
	file.close();
	return true;
}
