// cilent.cpp

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

    SOCKET clientSocket;

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

    clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Creating Socket" << std::endl;
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Socket Created" << std::endl;
    }


    sockaddr_in cAddr;

    // connection to port
    cAddr.sin_family = AF_INET;
    cAddr.sin_port = htons(port);
    cAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    // connection to server
    if (connect(clientSocket, (struct sockaddr*)&cAddr, sizeof(cAddr)) == SOCKET_ERROR) {
        std::cout << "Failed: Error Connecting To The Server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Connect To Server" <<std::endl;
    }


    // message sending 
    std::string sendMsg;
    char buf[256] = "";
    std::cout << "Please Type a Message for the server: ";

    std::getline(std::cin,sendMsg);

    int byteSent = send(clientSocket, sendMsg.c_str(), sendMsg.length(), 0);

    if (byteSent == SOCKET_ERROR) {
        std::cout << "Failed: Error Sending Data to Server" << std::endl;

        WSACleanup();
        return 1;
    } else {
        //std::cout << "Ok: " << byteSent << " bytes to the server: " << sendMsg << std::endl;
    }


    // recvinging acknolwedgement from server 
    byteSent = recv(clientSocket, buf, sizeof(buf), 0);
    if (byteSent <= 0) {
        std::cout << "Failed: Error Receiving Data from the Client" << std::endl;

        WSACleanup();
        return 1;

    } else {
        // Send acknowledgment back to the client
        sendMsg.assign(buf,byteSent);

        std::cout << "Server Ack: " << sendMsg;

    }
    

    WSACleanup();

    return 0;

}