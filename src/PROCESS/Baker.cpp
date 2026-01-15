#include <unistd.h>
#include <iostream>
#include <signal.h>

static void handler_signalone(int sig) {
	std::cout << "TEST\n";
	return;
}

int main() {
	struct sigaction sa;
	sa.sa_handler = handler_signalone;
	sigaction(SIGUSR2, &sa, NULL);
	
	std::cout << "TEST2\n";

	sleep(10);
	
	return 0;
}
