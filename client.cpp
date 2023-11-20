// client.cpp

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <future>   // access to std::async

#include <sstream>
#include <string>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")
#define port 5000




std::string setUser()
{
    std::stringstream ss;
    std::string input; 
    std::string userName;
    std::string temp;
    std::cout << "Welcome To the Chatroom: \n" 
    << "Please Enter a UserName to Chat: ";
    std::getline(std::cin,input);

    ss << input;

    // Loop to remove spaces
    while (!ss.eof()) {
        ss >> temp;
        userName = userName + temp;
    }
    
    return userName;
}


int main(int argc, char const* argv[])
{
    WSADATA wsaData;
    SOCKET clientSocket;
    std::string userName = setUser();

    // future async
    std::future<void> readingInput;

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



    std::cout << "Myusername is: [" << userName << "].";


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



        // Received The Welcome Message
        char buf[256] = "";
        std::string sendMsg = "";

        // recvinging acknolwedgement from server 
        int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
    
        if (bytesCount <= 0) {

            closesocket(clientSocket);
            WSACleanup();
            //std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError()  << std::endl;
            //WSACleanup();
        } else {
            // Send acknowledgment back to the client
            sendMsg.assign(buf,bytesCount);
            std::cout << sendMsg << std::endl;
        }




    struct timeval timeout;
    u_long iMode=1;
    ioctlsocket(clientSocket,FIONBIO,&iMode);

    bool keepGoing = true;
   



    fd_set readFD;
    fd_set writeFD;
    FD_ZERO(&readFD);
    FD_ZERO(&writeFD);
    FD_SET(clientSocket,&readFD);
    FD_SET(clientSocket,&writeFD);


 
    // Async Operation to recv input from server idependently
    readingInput = std::async(std::launch::async, [&]() {
        while (keepGoing) {

            fd_set readDescript = readFD;

            SOCKET clientAmount = select(clientSocket + 1, &readDescript, NULL, NULL, NULL);

            if (clientAmount == -1)
            {
                // closes socket
                std::cout << "Nothing to read:" << std::endl;
                closesocket(clientAmount);
                FD_CLR(clientAmount, &readFD);
                WSACleanup();
                keepGoing = false;
                return;

            }
            else
            {
                if(FD_ISSET(clientSocket,&readFD))
                {
                    char buf[256] = "";
                    std::string sendMsg = "";
                    // recvinging acknolwedgement from server 
                    int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
                
                    if (bytesCount <= 0) {
                        // if there is no byte ignore
                        //std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError()  << std::endl;
                        //WSACleanup();
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
        fd_set readDescript = readFD;
        fd_set writeDescript = writeFD;

        SOCKET clientAmount = select(clientSocket + 1, &readDescript, &writeDescript, NULL, NULL);

        if(clientAmount == -1)
        {
            std::cout << "CLIENTAMOUTN ERROR: " <<WSAGetLastError() << std::endl;
            //WSACleanup();

        }
        else
        {
        if(FD_ISSET(clientSocket,&readFD))
        {

            char buf[256] = "";
            std::string sendMsg = "";

            // recvinging acknolwedgement from server 
            int bytesCount = recv(clientSocket, buf, sizeof(buf), 0);
        
            if (bytesCount <= 0) {

                closesocket(clientAmount);
                FD_CLR(clientAmount, &readDescript);
                //std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError()  << std::endl;
                //WSACleanup();
            } else {
                // Send acknowledgment back to the client
                sendMsg.assign(buf,bytesCount);
                std::cout << sendMsg << std::endl;
            }
        }

        if(FD_ISSET(clientSocket,&writeFD))
        {
            std::string sendMsg = "";
            std::getline(std::cin,sendMsg);

            int byteSent = send(clientSocket, sendMsg.c_str(), sendMsg.length(), 0);
            if (sendMsg == "/quit")
            {
                closesocket(clientSocket);
                WSACleanup();
                return 1;
            }

            if (byteSent == SOCKET_ERROR) {
                std::cout << "Failed: Error Sending Data to Server" << WSAGetLastError()<< std::endl;
                WSACleanup();
                return 1;
            } else {
                //std::cout << "Ok: " << byteSent << " bytes to the server: " << sendMsg << std::endl;
            }
        }

        }

    }   // end while 

    // wait til sync with readInput
    readingInput.wait();



    //bool keepGoing = true;
    bool write = true;

    while(keepGoing)
    {
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;

}

