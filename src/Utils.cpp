#include "../include/Utils.h"
#include "../include/Config.h"

bool UTILS::setupKeyFile(const char* path) {
	FILE* file = fopen(path, "a");
	
	if(!file) 
		return false;
		
	fclose(file);
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

int UTILS::SEM_INDEX_MUTEX(int id) {
	return (static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) + 3 * (id) + 1);
}

int UTILS::SEM_INDEX_SLOTS(int id) {
	return (static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) + 3 * (id) + 2);
}

int UTILS::SEM_INDEX_COUNT(int id) {
	return (static_cast<int>(SemaphoreTypes::PRODUCTS_BASE) + 3 * (id) + 3);
}
