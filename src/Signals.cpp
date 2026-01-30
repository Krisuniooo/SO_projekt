#include "Signals.h"
#include <csignal>
#include <iostream>
#include <cstring>
#include <errno.h>

void SIGNALS::init(int signal) {
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, signal);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("SIGNALS init_mask failed");
	}
}

void SIGNALS::send(int pid, int signal) {
	if(kill(pid, signal) == -1) {
		if(errno == ESRCH) {
			return;
		} else {
			std::cerr << "signal error " << errno << "\n";
		}
	}
}

void SIGNALS::wait(int signal) {
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, signal);
	int result = sigwaitinfo(&mask, NULL);

	if(result == -1) {
		if(errno != EINTR) {
			std::cerr << "SIGNALS wait error: " << strerror(errno) << "\n";
		}
	}
}
