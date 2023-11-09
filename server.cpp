// server.cpp

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define port 5000


int main(int argc, char const* argv[])
{
    WSADATA wsaData;

    SOCKET serverSocket;
    SOCKET acceptSocket;


    // initialize wsastartup
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Initialized Winsock:" << std::endl;
        std::cout << "Status: " << wsaData.szSystemStatus <<std::endl;
    }

    // create TCP socket 
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Creating Socket" << std::endl;
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Socket Created" << std::endl;
    }

    // The server port connection 5000
    sockaddr_in sAddr;
    sAddr.sin_family = AF_INET;
    sAddr.sin_port = htons(port); 
    sAddr.sin_addr.s_addr = INADDR_ANY;

    // binds to serverPort
    if (bind(serverSocket, (struct sockaddr*)&sAddr, sizeof(sAddr)) == SOCKET_ERROR) {
        std::cout << "Failed: Error On Bind Socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }else
    {
        std::cout << "Ok: Bind" << std::endl;
    }

    // listening to 2
    if (listen(serverSocket, 2) == SOCKET_ERROR) {
        std::cout << "Failed: Error On Listening Socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Listen" << std::endl;
    }


    // accepting server
    acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Accepting Socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Accepted Socket" << std::endl;
    }


    // message reciving 
    std::string recvMsg;
    char buf[256] = "";
    int byteCount = recv(acceptSocket, buf, sizeof(buf), 0);

    if (byteCount <= 0) {
        std::cout << "Failed: Error Receiving Data from the Client" << std::endl;

        WSACleanup();
        return 1;

    } else {
        // Send acknowledgment back to the client
        recvMsg.assign(buf, byteCount);

        //char ackBuf[256] = ""; 
        //std::cout <<  "Message received!" << std::endl;

        //byteCount = (acceptSocket, ack.c_str(), ack.length(), 0);

    }

    
    // sending Acknowlegement 
    char ackBuf[256] = "Message received!";

    byteCount = send(acceptSocket, ackBuf, sizeof(ackBuf), 0);

    if (byteCount <= 0) {
        std::cout << "Failed: Error Sending ACK" << std::endl;

        WSACleanup();
        return 1;

    } else {
        // Send acknowledgment back to the client
        std::cout << "Ok: Acknowledgement Sent to Client" << std::endl;
    }

    std::cout << "Cilent: " << recvMsg << std::endl;

    WSACleanup();

    return 0;


}