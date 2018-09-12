#include <thread>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include "src/TunConnector.h"

enum MODE : int {CLIENT=-1, SERVER=1, BOTH=0};

/**
 * Method to print the help
 */
void usage() {
    const char message[] =
            "\n"
            "Example of usage:\n"
            "\t sudo ./TunConnector [OPTIONS]\n"
            "--serverinterface   -p X    To set the name if the interface used by the server component\n"
            "--clientinterface   -n X    To set the name if the interface used by the client component\n"
            "--serverip          -s X    To set the ip on the server interface\n"
            "--clientip          -c X    To set the ip on the client interface\n"
            "--outport           -o X    To set the port used by the client component. Default 55555\n"
            "--inport            -i X    To set the port used by the server component. Default 55556\n"
            "--remoteip          -r X    To set the ip of the remote server to connect to\n"
            "--mode              -m [client|server|both] To set run mode. Default both\n"
            "--help              -h      Show this message\n";
    std::cout <<message<<std::endl;
}

/**
 * @param a char to be compared
 * @param b char to be compared
 * @return true if the char are equals ignoring the case, false otherwise
 */
bool ignore_case_char_compare(unsigned char a, unsigned char b) {
    return std::tolower(a) == std::tolower(b);
}

/**
 * @param a string to be compared
 * @param b string to be compared
 * @return true if the strings are equals ignoring the case, false otherwise
 */
bool ignore_case_string_compare(std::string const& a, std::string const& b) {
    if (a.length()==b.length())
        return std::equal(b.begin(), b.end(), a.begin(), ignore_case_char_compare);
    return false;
}

int main(int argc, char *argv[]) {
    // default parameters
    std::string if0 = "tun1";
    std::string if1 = "tun0";
    std::string ip0 = "192.168.1.2";
    std::string ip1 = "192.168.0.2";
    std::string remote_ip = "192.168.29.33";
    unsigned short int client_port = 55555;
    unsigned short int server_port = 55556;
    MODE mode = BOTH;

    // parse the opt arguments
    int c = 0;
    while (c != -1) {
        static struct option long_options[] = {
            {"help",  no_argument, 0, 'h'},
            {"serverinterface",  required_argument, 0, 'p'},
            {"clientinterface",  required_argument, 0, 'n'},
            {"serverip",  required_argument, 0, 's'},
            {"clientip",    required_argument, 0, 'c'},
            {"remoteip",    required_argument, 0, 'r'},
            {"outport",    required_argument, 0, 'o'},
            {"inport",    required_argument, 0, 'i'},
            {"mode",    required_argument, 0, 'm'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:n:s:c:o:i:r:m:h", long_options, &option_index);

        switch (c) {
            case 'p':
                if0 = optarg;
                break;

            case 'n':
                if1 = optarg;
                break;

            case 's':
                ip0 = optarg;
                break;

            case 'c':
                ip1 = optarg;
                break;

            case 'o':
                client_port = static_cast<unsigned short>(atoi(optarg));
                break;

            case 'i':
                server_port= static_cast<unsigned short>(atoi(optarg));
                break;

            case 'r':
                remote_ip = optarg;
                break;

            case 'm':
                if (ignore_case_string_compare(optarg, "client"))
                    mode = CLIENT;
                else if (ignore_case_string_compare(optarg, "server"))
                    mode = SERVER;
                else if (ignore_case_string_compare(optarg, "both"))
                    mode = BOTH;
                else {
                    usage();
                    exit(1);
                }
                break;

            case 'h':
                usage();
                exit(0);
                break;

            case '?':
                usage();
                exit(1);
                break;

            default:
                break;
        }
    }

    std::vector<std::thread> threads;

    // creating lambdas to be called to create the connections
    auto funcserver = [](std::string if_name,
                         unsigned short int port,
                         std::string local_ip) -> void {
        connection::TunConnector(if_name, local_ip, port).simpletunserver();
    };

    auto funcclient = [](std::string if_name,
                         std::string remote_ip,
                         unsigned short int port,
                         std::string local_ip) -> void {
        connection::TunConnector(if_name, local_ip, port).simpletunclient(remote_ip);
    };

    // based on optargs create the threads
    if (mode == BOTH || mode == SERVER) {
        threads.emplace_back(std::thread(funcserver, if0, server_port, ip0));
    }

    if (mode == BOTH || mode == CLIENT) {
        threads.emplace_back(std::thread(funcclient, if1, remote_ip, client_port, ip1));
    }

    // starting the program
    for(auto& t : threads) {
        t.join();
    }
}