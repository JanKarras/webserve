#include "../../include/webserv.hpp"

bool running = true;

void handle_sigint(int sig, siginfo_t *siginfo, void *context) {
	(void)sig;
	(void)siginfo;
	(void)context;
	running = false;
}

bool initSignal(void) {
	struct sigaction sa;
	sa.sa_sigaction = handle_sigint;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) {
		std::cerr << "SigInt error!" << std::endl;
		return (false);
	}
	return (true);
}
