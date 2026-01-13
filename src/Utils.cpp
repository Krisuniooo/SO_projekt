#include "../include/Utils.h"

#include <fstream>
#include <string>
#include <random>
#include <type_traits>

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

int UTILS::getRandom(int min, int max) {
	if(min>max) return -1;
	std::random_device dev;
	std::mt19937 rng(dev());
	
	std::uniform_int_distribution<int> dist(min, max);

	return dist(rng);
}

float UTILS::getRandom(float min, float max) {
	if(min>max) return -1;
	std::random_device dev;
	std::mt19937 rng(dev());
	
	std::uniform_real_distribution<float> dist(min, max);
	
	return dist(rng);
}

double UTILS::getRandom(double min, double max) {
	if(min>max) return -1;
	std::random_device dev;
	std::mt19937 rng(dev());
	
	std::uniform_real_distribution<double> dist(min, max);
	
	return dist(rng);
}
