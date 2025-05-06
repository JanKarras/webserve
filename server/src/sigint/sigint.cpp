#include "../../include/webserv.hpp"

bool running = true;

void handle_sigint(int sig, siginfo_t *siginfo, void *context) {
	(void)sig;
	(void)siginfo;
	(void)context;
	running = false;
}

void handleSigPipe(int signal) {
	//std::cout << signal << std::endl;
}

bool initSignal(void) {
	struct sigaction sa;
	sa.sa_sigaction = handle_sigint;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	signal(SIGPIPE, handleSigPipe);
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		Logger::error("Failed to initalize ctrl + c signal");
		return (false);
	}
	return (true);
}



