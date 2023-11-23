// client.cpp

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <future>  

#include <sstream>
#include <string>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#define port 5000

int main(int argc, char const* argv[])
{
    WSADATA wsaData;
    SOCKET clientSocket;

    std::future<void> readingInput;
    std::string theUser;

    // initialize wsastartup
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock" << std::endl;
        return -1;
    }
    else
    {
        std::cout << "Initialized Winsock:" << std::endl;
        std::cout << "Status: " << wsaData.szSystemStatus <<std::endl;
    }

    // initialize tcp sockets
    clientSocket = INVALID_SOCKET;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Creating Socket" << std::endl;
        WSACleanup();
        return -1;
    }
    else
    {
        std::cout << "Ok: Socket Created" << std::endl;
    }

   
    // connection to port
    sockaddr_in cAddr;
    cAddr.sin_family = AF_INET;
    cAddr.sin_port = htons(port);
    cAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // connection to server
    if (connect(clientSocket, (struct sockaddr*)&cAddr, sizeof(cAddr)) == SOCKET_ERROR) {
        std::cout << "Failed: Error Connecting To The Server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }
    else
    {
        std::cout << "Ok: Connect To Server" <<std::endl;
    }

    // Received The Welcome Message
    char buf[256] = "";
    std::string sendMsg = "";

    std::stringstream ss;
    std::string input; 
    std::string userName;
    std::string temp;

    // receive acknowledge from server 
    int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
    if (bytesCount <= 0) {
        closesocket(clientSocket);
        WSACleanup();
        std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError()  << std::endl;
    } else {
        // Send acknowledgment back to the client 
        sendMsg.assign(buf,bytesCount);

        // Print Msg Send from server
        std::cout << sendMsg << std::endl;

        // send to server set username
        ss.str("");
        ss.clear();
        std::getline(std::cin,input);
        ss << input;
        // Loop to remove spaces
        while (!ss.eof()) {
            ss >> temp;
            userName = userName + temp;
        }
        int byteSent = send(clientSocket, userName.c_str(), userName.length(), 0);
        
    }

    // set socket into nonblocking mode
    u_long unblock = 1;
    ioctlsocket(clientSocket,FIONBIO,&unblock);

    bool keepGoing = true;

    // create FileDescriptors 
    fd_set readFD;
    fd_set writeFD;
    FD_ZERO(&readFD);
    FD_ZERO(&writeFD);
    FD_SET(clientSocket,&readFD);
    FD_SET(clientSocket,&writeFD);
 
 
    // create async to keep reading for input 
    readingInput = std::async(std::launch::async, [&]() {
        while (keepGoing) 
        {
            fd_set readDescript = readFD;
            SOCKET clientAmount = select(clientSocket + 1, &readDescript, NULL, NULL, NULL);
            
            if (clientAmount == -1)
            {
                // closes socket
                std::cout << "Nothing to read:" << std::endl;
                closesocket(clientSocket);
                FD_CLR(clientSocket, &readFD);
                WSACleanup();
                keepGoing = false;
                return;
            }
            else
            {
                // Reads server message if sent
                if(FD_ISSET(clientSocket,&readFD))
                {
                    char buf[256] = "";
                    std::string sendMsg = "";
                    int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
                
                    if (bytesCount <= 0) {
                        // if there is no byte sent ignore
                    } else {
                        // Send acknowledgment back to the client
                        sendMsg.assign(buf,bytesCount);
                        std::cout << sendMsg;
                    }
                }  
            }

        }   // while loop
    });

    while(keepGoing)
    {
        // create tempFDs
        fd_set readDescript = readFD;
        fd_set writeDescript = writeFD;

        // select socket that is reading or writing
        SOCKET clientAmount = select(clientSocket + 1, &readDescript, &writeDescript, NULL, NULL);

        if(clientAmount == -1)
        {
            std::cout << "Client ERROR: " <<WSAGetLastError() << std::endl;
            keepGoing = false;
        }
        else
        {
            // Check If Recv from server
            if(FD_ISSET(clientSocket,&readFD))
            {
                char buf[256] = "";
                std::string sendMsg = "";

                // receiving acknowledgement from server 
                int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
            
                if (bytesCount <= 0) {
                    closesocket(clientAmount);
                    FD_CLR(clientAmount, &readDescript);
                } else {
                    // Send acknowledgment back to the client
                    sendMsg.assign(buf,bytesCount);
                    std::cout << sendMsg << std::endl;
                }
            }   // end readFD

            // Check If 
            if(FD_ISSET(clientSocket,&writeFD))
            {
                std::string sendMsg = "";
                std::getline(std::cin,sendMsg);

                if (sendMsg == "/quit")
                {
                    closesocket(clientSocket);
                    WSACleanup();
                    return -1;
                }

                int byteSent = send(clientSocket, sendMsg.c_str(), sendMsg.length(), 0);

                if (byteSent == SOCKET_ERROR) {
                    std::cout << "Failed: Error Sending Data to Server: " << WSAGetLastError()<< std::endl;
                    closesocket(clientSocket);
                    WSACleanup();
                    keepGoing = false;
                    return -1;
                } else {
                    // bytes sent 
                }
            }   // end writeFD
        }
    }   // end while 

    
    closesocket(clientSocket);
    WSACleanup();

    return -1;

}