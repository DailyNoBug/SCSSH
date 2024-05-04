//
// Created by lsc on 24-5-3.
//

#ifndef SCSSH_SSH_H
#define SCSSH_SSH_H

#include <libssh2.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glog/logging.h>

class SSHClient {
private:
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;

public:
    SSHClient(const char *hostname, int port, const char *username, const char *password);
    ~SSHClient();
    std::string execute(const std::string& command);
    std::string execute_with_timeout(const std::string& command, int timeout_seconds);
};

#endif //SCSSH_SSH_H
