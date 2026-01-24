#ifndef UTILS_H
#define UTILS_H

namespace UTILS {
	bool setupKeyFile(const char* path);
	
	void getTimestamp(char* buffer, const unsigned int bufferSize);
	
	int getRandom(int min, int max);
	float getRandom(float min, float max);
	double getRandom(double min, double max);
	
	int SEM_INDEX_MUTEX(int id);
	int SEM_INDEX_SLOTS(int id);
	int SEM_INDEX_COUNT(int id);
}

#endif
