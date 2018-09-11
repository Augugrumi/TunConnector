//
// Created by zanna on 11/09/18.
//

#ifndef CONNECTION_TUNCONNECTOR_H_
#define CONNECTION_TUNCONNECTOR_H_

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <boost/log/trivial.hpp>
#include <linux/if_tun.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>

#define BUFSIZE 4096
#define BOOST_LOG_DYN_LINK 1

typedef int fd_type;

namespace connection{

    class TunConnector {
    private:
        const std::string _if_name;
        const std::string _local_ip;
        const unsigned short int _port;

        void my_exit(int code);
        int tun_alloc(char *dev, int flags, const std::string& component = "");
        int cread(int fd, char *buf, int n, const std::string& component = "");
        int cwrite(int fd, char *buf, int n, const std::string& component = "");
        int read_n(int fd, char *buf, int n, const std::string& component = "");
        fd_type allocate_tunnel_by_string_name(const std::string& name, const std::string& component = "");
        fd_type open_socket(const std::string& component = "");
        void assign_port(struct sockaddr_in& socket, int ip, int port);
        void open_tunnel(fd_type tap_fd, fd_type net_fd, const std::string& component = "");
        bool check_ip_address(const std::string& ip);
        void setup_interface();
        void destroy_interface();

    public:
        TunConnector(const std::string& if_name, const std::string& local_ip, unsigned short int port);
        void simpletunserver();
        void simpletunclient(const std::string& remote_ip);
        ~TunConnector();
    }; // class TunConnector

} // namespace connection

#endif //CONNECTION_TUNCONNECTOR_H_