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

// default buffer size -> TODO think how to check it
#define BUFSIZE 4096
// for logging purposes
#define BOOST_LOG_DYN_LINK 1

typedef int fd_type;

namespace connection{

    class TunConnector {
    private:
        /**
         * Name of the interface used for the tunnel
         */
        const std::string _if_name;
        /**
         * IP assigned on the interface used for the tunnel
         */
        const std::string _local_ip;
        /**
         * port used for the tunnel
         */
        const unsigned short int _port;


        /**
         * Method used in case of errors
         * @param code int error code
         */
        void my_exit(int code);

        /**
         * Method that allocates or reconnects to a tun/tap device. The caller must reserve enough space in *dev
         * @param dev Name of the interface
         * @param flags Flags used to create the tunnel (IFF_TUN | IFF_NO_PI)
         * @param component String used for debugging purposes
         * @return identifier of the file descriptor used by the tunnel
         */
        int tun_alloc(char *dev, int flags, const std::string& component = "");

        /**
         * Utility method to read from a file descriptor and save the result on the buffer, on error it stops the program
         * @param fd Identifier of the file descriptor
         * @param buf Buffer to be filled with the data read
         * @param n Max length of data to be read
         * @param component String used for debugging purposes
         * @return Length of data actually read
         */
        int cread(int fd, char *buf, int n, const std::string& component = "");

        /**
         * Utility method to ensure we read exactly n bytes, and puts them into a buffer
         * @param fd Identifier of the file descriptor
         * @param buf Buffer to be filled with the data read
         * @param n Max length of data to be read
         * @param component String used for debugging purposes
         * @return Length of the next packet to be read
         */
        int read_n(int fd, char *buf, int n, const std::string& component = "");

        /**
         * Utility method to write data stored in a buffer to a file descriptor, on error it stops the program
         * @param fd Identifier of the file descriptor
         * @param buf Buffer with data
         * @param n Size of the buffer
         * @param component String used for debugging purposes
         * @return Length of data actually written
         */
        int cwrite(int fd, char *buf, int n, const std::string& component = "");

        /**
         * Auxiliary method to call tun_alloc setting the right values
         * @param name Name of the interface to use
         * @param component String used for debugging purposes
         * @return identifier of the file descriptor used by the tunnel
         */
        fd_type allocate_tunnel_by_string_name(const std::string& name, const std::string& component = "");

        /**
         * Method to open a socket
         * @param component String used for debugging purposes
         * @return The file descriptor associate with the socket
         */
        fd_type open_socket(const std::string& component = "");

        /**
         * Method to set the port used by the interface for the communications
         * @param socket Open socket used for communications
         * @param ip Remote IP of the server in case of client, 0.0.0.0 in case of server
         * @param port Port to be assigned
         */
        void assign_port(struct sockaddr_in& socket, int ip, int port);

        /**
         * Method that actually establish the connection
         * @param tap_fd File descriptor that permits to communicate with the tunnel
         * @param net_fd File descriptor that permits the communication on the network
         * @param component String used for debugging purposes
         */
        void open_tunnel(fd_type tap_fd, fd_type net_fd, const std::string& component = "");

        /**
         * Utility method to check if a string represent a valid IPv4 IP address
         * @param ip String that represent an IP address
         * @return true if the string is a valid IP address, false otherwise
         */
        bool check_ip_address(const std::string& ip);

        /**
         * Method that create the interface, setting the IP
         * TODO think a better approach
         */
        void setup_interface();

        /**
         * Method that destroy the interface used, called in case of errors by my_exit
         * TODO think how to call it on program end
         */
        void destroy_interface();

    public:
        /**
         * Constructor of TunConnector
         * @param if_name Name of the interface to be used
         * @param local_ip IP used inside the Tunnel
         * @param port Port to be used by the tunnel
         */
        TunConnector(const std::string& if_name, const std::string& local_ip, unsigned short int port);

        /**
         * Method to be called to create a server on the tunnel
         */
        void simpletunserver();

        /**
         * Method to be called to create a client on the tunnel
         * @param remote_ip "Real" IP address of the remote server
         */
        void simpletunclient(const std::string& remote_ip);

        /**
         * Destructor
         */
        ~TunConnector();
    }; // class TunConnector

} // namespace connection

#endif //CONNECTION_TUNCONNECTOR_H_