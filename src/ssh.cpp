//
// Created by lsc on 24-5-3.
//

#include "ssh.h"
#include <iostream>
static bool libssh2_setup = false;

SSHClient::SSHClient(const char *hostname, int port, const char *username, const char *password) {

    int rc = 0;
    if (!libssh2_setup) {
        libssh2_setup = true;
        rc = libssh2_init(0);
        if (rc != 0) {
            LOG(ERROR) << "libssh2 initialization failed ("
                << rc << ")" << username << "@" << hostname << ":" << port;
            return ;
        }
    }
    LOG(INFO) << "libssh2 initialization success" << username << "@" << hostname << ":" << port;

    session = libssh2_session_init();
    if (!session)
    {
        LOG(ERROR) << "libssh2 session initialization failed "
            << username << "@" << hostname << ":" << port;
        return ;
    }
    LOG(INFO) << "libssh2 session initialization success "
        << username << "@" << hostname << ":" << port;

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        LOG(ERROR) << "Failed to create socket " << username << "@" << hostname << ":" << port;
        return ;
    }
    LOG(INFO) << "socket created: " << socket_fd << " " << username << "@" << hostname << ":" << port;

    // set server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, hostname, &server_addr.sin_addr);

    // connect to server
    rc = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc != 0)
    {
        LOG(ERROR) << "Failed to connect to server " << username << "@" << hostname << ":" << port;
        close(socket_fd);
        return ;
    }
    LOG(INFO) << "connected to server " << username << "@" << hostname << ":" << port;

    // set session options
    libssh2_session_set_blocking(session, 1);

    // connect to the SSH server
    rc = libssh2_session_handshake(session, socket_fd);
    if (rc == 0)
    {
        LOG(INFO) << "libssh2 session handshake success";
    }
    else if (rc == LIBSSH2_ERROR_EAGAIN)
    {
        LOG(ERROR) << "need more info of sesion: " << rc;
        return ;
    }
    else
    {
        LOG(ERROR) << "libssh2 session handshake failed: " << rc;
        return ;
    }

    // authenticate via password
    rc = libssh2_userauth_password(session, username, password);
    if (rc != 0)
    {
        LOG(ERROR) << "libssh2 user authentication failed: " << rc;
        return ;
    }
    LOG(INFO) << "libssh2 user authentication success";
}

SSHClient::~SSHClient() {
    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);
    libssh2_exit();
}

std::string SSHClient::execute(const std::string& command) {
    std::string result;

    channel = libssh2_channel_open_session(session);
    if (channel == NULL)
    {
        LOG(ERROR) << "libssh2 channel open failed";
        return "libssh2 channel open failed";
    }
    LOG(INFO) << "libssh2 channel open success";

    int rc = libssh2_channel_exec(channel, command.c_str());
    if (rc < 0) {
        LOG(ERROR) << "Error writing command to channel";
        return "Error writing command to channel";
    }

    char buffer[1024];
    int nbytes;
    do {
        nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer));
        LOG(INFO) << "nbytes: " << nbytes;
        if (nbytes > 0) {
            result.append(buffer, nbytes);
        } else if (nbytes < 0) {
            LOG(ERROR) << "Error reading from channel";
            return "Error reading from channel";
        }
    } while (nbytes > 0);

    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
    return result;
}

std::string SSHClient::execute_with_timeout(const std::string& command, int timeout_seconds) {
    std::string result;

    channel = libssh2_channel_open_session(session);
    if (channel == NULL) {
        LOG(ERROR) << "channel open failed";
        return "channel open failed";
    }
    LOG(INFO) << "channel open success";

    int rc = libssh2_channel_exec(channel, command.c_str());
    if (rc < 0) {
        LOG(ERROR) << "channel write error";
        return "channel write error";
    }

    auto start_time = std::chrono::steady_clock::now();
    char buffer[1024];
    int nbytes;
    do {
        nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer));
        LOG(INFO) << "nbytes：" << nbytes;
        if (nbytes > 0) {
            result.append(buffer, nbytes);
        } else if (nbytes < 0) {
            LOG(ERROR) << "read from channel error";
            return "read from channel error";
        }

        auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count();
        if (elapsed_seconds >= timeout_seconds) {
            LOG(ERROR) << "channel execute timeout";
            libssh2_channel_close(channel);
            LOG(ERROR) << "ssh channel closed";
            std::this_thread::sleep_for(std::chrono::seconds(1));
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    } while (nbytes > 0);

    while (true) {
        nbytes = libssh2_channel_read(channel, buffer, sizeof(buffer));
        if (nbytes > 0) {
            result.append(buffer, nbytes);
            LOG(INFO) << "nbytes：" << nbytes;
        } else {
            break;
        }
    }

    libssh2_channel_close(channel);
    libssh2_channel_free(channel);
    return result;
}