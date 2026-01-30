#ifndef SIGNALS_H
#define SIGNALS_H

#include <csignal>
#include <iostream>
#include <errno.h>
#include <unistd.h>

namespace SIGNALS {
	void init(int signal);
	void send(int pid, int signal);
	void wait(int signal);
}

#endif
