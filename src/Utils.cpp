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

void UTILS::getTimestamp(char* buffer, const unsigned int bufferSize) {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
}
