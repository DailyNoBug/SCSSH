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
#include "ssh.h"
#include <glog/logging.h>

int main(int argc, char *argv[])
{
    int rc = 0;

    FLAGS_alsologtostderr = true;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_minloglevel = google::INFO;
    FLAGS_log_dir = "./SCLOG";
    google::InitGoogleLogging(argv[0]);

    SSHClient cli = SSHClient("#", 22, "#", "#");
    LOG(INFO) << "SSHClient created ";

    std::string result = cli.execute("ls");
    std::cout << result << std::endl;

    result = cli.execute("pwd");
    std::cout << result << std::endl;

    result = cli.execute("cd frp && ls");
    std::cout << result << std::endl;

    result = cli.execute_with_timeout("ping baidu.com", 5);
    std::cout << result << std::endl;

    google::ShutdownGoogleLogging();
}