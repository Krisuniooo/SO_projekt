#include <unistd.h>
#include <iostream>
#include <signal.h>
#include "../../include/Config.h"
#include "../../include/Utils.h"

static void handlerSigOne(int sig) {
	std::cout << "TEST\n";
	
	return;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handlerSigOne;
	sigaction(SIGUSR2, &sa, NULL);
	
	std::cout << "TEST2\n";

	while(true) {
		int time = UTILS::getRandom(BAKE_MIN_TIME, BAKE_MAX_TIME) * SIMULATION_MINUTE;
		usleep(time);
		
		//todo
		break;
	}
	
	return 0;
}
