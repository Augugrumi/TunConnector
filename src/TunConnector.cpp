//
// Created by zanna on 11/09/18.
//

#include "TunConnector.h"

namespace connection{

    void TunConnector::my_exit(int code) {
        destroy_interface();
        exit(code);
    }

    int TunConnector::tun_alloc(char *dev, int flags, const std::string& component) {
        struct ifreq ifr;
        int fd, err;
        const char *clonedev = "/dev/net/tun";

        if( (fd = open(clonedev , O_RDWR)) < 0 ) {
            BOOST_LOG_TRIVIAL(error) << component << "Error opening /dev/net/tun";
            return fd;
        }

        memset(&ifr, 0, sizeof(ifr));

        ifr.ifr_flags = flags;

        if (*dev) {
            strncpy(ifr.ifr_name, dev, IFNAMSIZ);
        }

        if( (err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0 ) {
            BOOST_LOG_TRIVIAL(error) << component << "Error on ioctl(TUNSETIFF)";
            close(fd);
            return err;
        }

        strcpy(dev, ifr.ifr_name);

        return fd;
    }

    int TunConnector::cread(int fd, char *buf, int n, const std::string& component){
        int nread;

        if((nread=read(fd, buf, n)) < 0){
            BOOST_LOG_TRIVIAL(fatal) << component <<"Error on reading data";
            my_exit(1);
        }
        return nread;
    }

    int TunConnector::cwrite(int fd, char *buf, int n, const std::string& component){
        int nwrite;

        if((nwrite=write(fd, buf, n)) < 0){
            BOOST_LOG_TRIVIAL(fatal) << component <<"Error on writing data";
            my_exit(1);
        }/* else {
            for(int i = 0; i < nwrite ; i++) {
                BOOST_LOG_TRIVIAL(fatal) << component << "buffer[" << i << "] = " << buf[i];
            }
        }*/
        return nwrite;
    }

    int TunConnector::read_n(int fd, char *buf, int n, const std::string& component) {
        int nread, left = n;

        while(left > 0) {
            if ((nread = cread(fd, buf, left, component)) == 0){
                return 0 ;
            }else {
                left -= nread;
                buf += nread;
            }
        }
        return n;
    }

    fd_type TunConnector::allocate_tunnel_by_string_name(const std::string& name, const std::string& component) {
        fd_type tap_fd;

        char *cstr = new char[name.length() + 1];
        strcpy(cstr, name.c_str());
        if ((tap_fd = tun_alloc(cstr, IFF_TUN | IFF_NO_PI, component)) < 0 ) {
            BOOST_LOG_TRIVIAL(fatal) << component << "Error connecting to tun/tap interface " << name;
            my_exit(1);
        }
        return tap_fd;
    }

    fd_type TunConnector::open_socket(const std::string& component) {
        fd_type sock_fd;
        if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << component << "Error opening socket";
            my_exit(1);
        }
        return sock_fd;
    }

    void TunConnector::assign_port(struct sockaddr_in& socket, int ip, int port) {
        memset(&socket, 0, sizeof(socket));
        socket.sin_family = AF_INET;
        socket.sin_addr.s_addr = ip;
        socket.sin_port = htons(port);
    }

    void TunConnector::open_tunnel(fd_type tap_fd, fd_type net_fd, const std::string& component) {
        char buffer[BUFSIZE];
        uint16_t plength, nread, nwrite;

        fd_type maxfd = (tap_fd > net_fd)?tap_fd:net_fd;

        while(1) {
            int ret;
            fd_set rd_set;

            FD_ZERO(&rd_set);
            FD_SET(tap_fd, &rd_set); FD_SET(net_fd, &rd_set);

            ret = select(maxfd + 1, &rd_set, NULL, NULL, NULL);

            if (ret < 0 && errno == EINTR){
                continue;
            }

            if (ret < 0) {
                BOOST_LOG_TRIVIAL(fatal) << component << "Error on select()";
                my_exit(1);
            }

            if(FD_ISSET(tap_fd, &rd_set)) {
                /* data from tun/tap: just read it and write it to the network */
                BOOST_LOG_TRIVIAL(debug) << component << "Reading data from the tunnel";
                nread = cread(tap_fd, buffer, BUFSIZE, component);

                /* write length + packet */
                BOOST_LOG_TRIVIAL(debug) << component << "Writing data into the tunnel";
                plength = htons(nread);
                nwrite = cwrite(net_fd, (char *)&plength, sizeof(plength), component);
                nwrite = cwrite(net_fd, buffer, nread, component);
            }

            if(FD_ISSET(net_fd, &rd_set)) {
                /* data from the network: read it, and write it to the tun/tap interface.
                 * We need to read the length first, and then the packet */
                BOOST_LOG_TRIVIAL(debug) << component << "Reading data from the network";
                /* Read length */
                nread = read_n(net_fd, (char *)&plength, sizeof(plength), component);
                if(nread == 0) {
                    /* ctrl-c at the other end */
                    break;
                }

                /* read packet */
                nread = read_n(net_fd, buffer, ntohs(plength), component);

                BOOST_LOG_TRIVIAL(debug) << component << "Writing data to the network";
                /* now buffer[] contains a full packet or frame, write it into the tun/tap interface */
                nwrite = cwrite(tap_fd, buffer, nread, component);
            }
        }
    }

    bool TunConnector::check_ip_address(const std::string& ip) {
        struct sockaddr_in sa;
        int result = inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr));
        return result != 0;
    }

    void TunConnector::setup_interface() {
        system(std::string("openvpn --mktun --dev " + _if_name).c_str());
        system(std::string("ip link set " + _if_name + " up").c_str());
        system(std::string("ip addr add " + _local_ip + "/24 dev " + _if_name).c_str());
    }

    void TunConnector::destroy_interface() {
        system(std::string("ip addr del " + _local_ip + "/24 dev " + _if_name).c_str());
        system(std::string("ip link delete " + _if_name).c_str());
        system(std::string("openvpn --rmtun --dev " + _if_name).c_str());
    }

    TunConnector::TunConnector(const std::string& if_name, const std::string& local_ip,unsigned short int port) :
            _if_name(if_name), _local_ip(local_ip), _port(port) {

        if(_if_name.length() == 0) {
            BOOST_LOG_TRIVIAL(fatal) << "Interface name not specified";
            my_exit(1);
        }
        setup_interface();
    }

    TunConnector::~TunConnector() {
        // TODO when to call destroy at the end of the program?
        //destroy_interface();
    }

    void TunConnector::simpletunclient(const std::string& remote_ip){
        fd_type tap_fd, sock_fd, net_fd;
        struct sockaddr_in remote;
        const std::string component = "CLIENT: ";

        BOOST_LOG_TRIVIAL(info) << "CLIENT: Starting...";

        if(!check_ip_address(remote_ip)) {
            BOOST_LOG_TRIVIAL(fatal) << "CLIENT: Server IP " << remote_ip <<" is not valid";
            my_exit(1);
        }

        tap_fd = allocate_tunnel_by_string_name(_if_name, component);
        sock_fd = open_socket(component);

        assign_port(remote, inet_addr(remote_ip.c_str()), _port);

        /* connection request */
        if (connect(sock_fd, (struct sockaddr*) &remote, sizeof(remote)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "CLIENT: Error connecting to the server " << remote_ip;
            my_exit(1);
        }

        net_fd = sock_fd;
        BOOST_LOG_TRIVIAL(info) << "CLIENT: Connected to server " << inet_ntoa(remote.sin_addr);

        open_tunnel(tap_fd, net_fd, component);
    }

    void TunConnector::simpletunserver() {
        fd_type tap_fd, sock_fd, net_fd;;
        struct sockaddr_in local, remote;
        int optval = 1;
        socklen_t remotelen;
        const std::string component = "SERVER: ";

        BOOST_LOG_TRIVIAL(info) << "SERVER: Starting...";

        tap_fd = allocate_tunnel_by_string_name(_if_name, component);
        sock_fd = open_socket(component);

        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "SERVER: Error on setsockopt()";
            my_exit(1);
        }

        assign_port(local, htonl(INADDR_ANY), _port);

        if (bind(sock_fd, (struct sockaddr*) &local, sizeof(local)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "SERVER: Error on binding to port " << _port;
            my_exit(1);
        }

        if (listen(sock_fd, 5) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "SERVER: Error on listen on port " << _port;
            my_exit(1);
        }

        BOOST_LOG_TRIVIAL(info) << "SERVER: Listening on port " << _port << "...";

        /* wait for connection request */
        remotelen = sizeof(remote);
        memset(&remote, 0, remotelen);
        if ((net_fd = accept(sock_fd, (struct sockaddr*)&remote, &remotelen)) < 0) {
            BOOST_LOG_TRIVIAL(fatal) << "SERVER: Error accepting request";
            my_exit(1);
        }

        BOOST_LOG_TRIVIAL(info) << "SERVER: Client connected from " << inet_ntoa(remote.sin_addr);

        open_tunnel(tap_fd, net_fd, component);
    }

} // namespace connection
