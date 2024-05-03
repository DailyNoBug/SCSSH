//
// Created by lsc on 24-5-3.
//
#include "libssh2_setup.h"
#include "libssh2.h"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <iostream>
#include <glog/logging.h>

int main(int argc, char *argv[])
{
    int rc = 0;

    FLAGS_alsologtostderr = true;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_minloglevel = google::INFO;
    FLAGS_log_dir = "./SCLOG";
    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "libssh2 version: " << libssh2_version(0);

    // init libssh2
    rc = libssh2_init(0);
    if (rc != 0)
    {
        LOG(ERROR) << "libssh2 initialization failed (" << rc << ")";
        return 1;
    }
    LOG(INFO) << "libssh2 initialization success";

    // create a new session instance
    LIBSSH2_SESSION *session;
    session = libssh2_session_init();
    if (!session)
    {
        LOG(ERROR) << "libssh2 session initialization failed";
        return 1;
    }
    LOG(INFO) << "libssh2 session initialization success";

    const char *hostip = "...";
    int port = 22;
    const char *user = "...";
    const char *password = "...";

    // init socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        LOG(ERROR) << "Failed to create socket";
        return 1;
    }
    LOG(INFO) << "socket created: " << socket_fd;

    // set server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, hostip, &server_addr.sin_addr);

    // connect to server
    rc = connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (rc != 0)
    {
        LOG(ERROR) << "Failed to connect to server";
        close(socket_fd);
        return 1;
    }
    LOG(INFO) << "connected to server";

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
        return 1;
    }
    else
    {
        LOG(ERROR) << "libssh2 session handshake failed: " << rc;
        return 1;
    }

    // authenticate via password
    rc = libssh2_userauth_password(session, user, password);
    if (rc != 0)
    {
        LOG(ERROR) << "libssh2 user authentication failed: " << rc;
        return 1;
    }
    LOG(INFO) << "libssh2 user authentication success";

    LIBSSH2_CHANNEL *channel;
    channel = libssh2_channel_open_session(session);
    if (channel == NULL)
    {
        LOG(ERROR) << "libssh2 channel open failed";
        return 1;
    }
    LOG(INFO) << "libssh2 channel open success";

    rc = libssh2_channel_exec(channel, "ls");
    if (rc != 0)
    {
        LOG(ERROR) << "libssh2 channel exec failed: " << rc;
        return 1;
    }
    LOG(INFO) << "libssh2 channel exec success: " << rc;

    // read command output
    char buffer[4096];
    int bytes_read;
    while ((bytes_read = libssh2_channel_read(channel, buffer, sizeof(buffer))) > 0)
    {
        std::cout.write(buffer, bytes_read);
    }

    if (bytes_read < 0)
    {
        std::cerr << "Error reading command output" << std::endl;
        close(socket_fd);
        return 1;
    }

    libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
    libssh2_session_free(session);
    LOG(INFO) << "libssh2 session freed: " << session;
    close(socket_fd);
    LOG(INFO) << "sock disconnected: " << socket_fd;
    libssh2_exit();
    LOG(INFO) << "libssh2 session closed";
    google::ShutdownGoogleLogging();
}