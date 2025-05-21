#include "../../include/webserv.hpp"

void checkCgiTimeouts(ConfigData &data) {
    for (size_t i = 0; i < data.servers.size(); ++i) {
        server &srv = data.servers[i];
        ServerContext &ctx = srv.serverContex;
        std::map<int, pid_t>::iterator it = ctx.pids.begin();

        while (it != ctx.pids.end()) {
            int clientFd = it->first;
            pid_t childPid = it->second;

            std::map<int, HttpResponse>::iterator respIt = ctx.responses.find(clientFd);
            if (respIt != ctx.responses.end()) {
                long long now = getCurrentTime();
                if (now - respIt->second.startTime >= TIME_TO_KILL_CHILD) {
                    std::map<int, pid_t>::iterator toErase = it;
                    ++it;
                    if (ctx.fds.count(clientFd)) {
                        epoll_ctl(data.epollFd, EPOLL_CTL_DEL, ctx.fds[clientFd], NULL);
                        close(ctx.fds[clientFd]);
                        ctx.fds.erase(clientFd);
                    }
                    if (ctx.cgifds.count(clientFd)) {
                        epoll_ctl(data.epollFd, EPOLL_CTL_DEL, ctx.cgifds[clientFd], NULL);
                        close(ctx.cgifds[clientFd]);
                        ctx.cgifds.erase(clientFd);
                    }
                    kill(childPid, SIGKILL);
                    ctx.pids.erase(toErase);
                    handle504(respIt->second);
					//Logger::info("Child von clientFd: %i wurde wegen zeituberschreitung beendet", clientFd);
                    continue;
                }
            }
            ++it;
        }
    }
}
