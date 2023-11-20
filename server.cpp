// server.cpp

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#include <iostream>

#include <sstream>

#include <string>

#include <vector>
#include <unordered_map>


#pragma comment(lib, "Ws2_32.lib")

#define port 5000



struct connectedUser
{

    SOCKET socket;
    std::string userName;
    std::string passWord;

};


int main(int argc, char const* argv[])
{
    WSADATA wsaData;

    SOCKET serverSocket;
    SOCKET acceptSocket;


    std::vector<connectedUser> userList;

    std::unordered_map<SOCKET,connectedUser> userMap;
    std::unordered_map<std::string,connectedUser> nameMap;



    // initialize wsastartup
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize Winsock" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Initialized Winsock:" << std::endl;
        std::cout << "Status: " << wsaData.szSystemStatus <<std::endl;
    }   // end wsastartup


    // create TCP socket 
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);


    //u_long iMode=1;
    //ioctlsocket(serverSocket,FIONBIO,&iMode);

    


    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Creating Socket" << std::endl;
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Socket Created" << std::endl;
    }   // end create tcp socket


    // connection of socket to port 
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
    
    // listening of max pending of 2
    if (listen(serverSocket, 24) == SOCKET_ERROR) {
        std::cout << "Failed: Error On Listening Socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else
    {
        std::cout << "Ok: Listen" << std::endl;
    }


    // Create FD
    fd_set readFd;

    // clear socket from set
    FD_ZERO(&readFd);

    // add to set
    FD_SET(serverSocket,&readFd);

    bool keepGoing = true;

    const int TIMEOUT_SECONDS = 5;

    while(keepGoing)
    {
        // accept clients
        // send clients
        // recv clients 
        fd_set tempFD = readFd;


        timeval timeout;
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;


        // 0,  FD_SETSIZE 64
        SOCKET socketAmount = select(0, &tempFD,NULL,NULL,NULL);

        if(socketAmount == 0)
        {

            std::cout << "User disconnected." << std::endl;

        }



        for (int i = 0; i < socketAmount; i++)
        {
            SOCKET socketList = tempFD.fd_array[i];

            if(socketList == serverSocket)
            {
                // accept new socket
                SOCKET newClient = accept(serverSocket, NULL, NULL);

                // add to orignal file descriptor
                FD_SET(newClient,&readFd);

    
                char userMsg[256] = "Please Enter Username:";
                send(newClient, userMsg, sizeof(userMsg), 0);

                // create buffer to receive bytes
				char buf[256];
                ZeroMemory(buf,256);
        
                int bytesCount = recv(newClient, buf, sizeof(buf), 0);
                if (bytesCount <= 0) {
                    std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError() << std::endl;
					closesocket(newClient);
					FD_CLR(newClient, &readFd);
                    //WSACleanup();
                } else
                {
                    
                }

                // disconnect user if same username

                std::string userNameMsg = "";
                userNameMsg.assign(buf,bytesCount);
                connectedUser account;

                // check dupe username
                if(nameMap.find(userNameMsg) != nameMap.end())
                {
                    // dupe user found
                    std::cout << "dupe user found !: " << std::endl;
                    std::stringstream dss;
                    std::string disconnect;
                    dss << "Username:" << account.userName << " has already been taken!\n"
                    << "Please Use a Different UserName! Good Bye!\n";

                    disconnect = dss.str();
                    send(newClient, disconnect.c_str(), disconnect.length(), 0);

                    closesocket(newClient);
                    FD_CLR(newClient, &readFd);

                }else
                {
                    // setup struct
                    account.socket = newClient;
                    account.userName = userNameMsg;

                    userList.push_back(account);
                
                    userMap[newClient] = account;
                    nameMap[userNameMsg] = account;

                    // set username
                    std::cout << "User: " << userNameMsg << " Has Arrived" << std::endl;


                    std::stringstream welcomeStream;
                    std::string welcomeMsg;
                    welcomeStream << "Welcome " << account.userName << " to the chat room!\n";

                    welcomeMsg = welcomeStream.str();
                    send(newClient, welcomeMsg.c_str(), welcomeMsg.length(), 0);
                }
            }
            else
            {   
                // create buffer to receive bytes
				char buf[256];
                ZeroMemory(buf,256);
            
                int bytesCount = recv(socketList, buf, sizeof(buf), 0);

                if (bytesCount <= 0) {
                    std::cout << "Failed: Error Receiving Data from the Client: " << WSAGetLastError() << std::endl;
					closesocket(socketList);
					FD_CLR(socketList, &readFd);
                    //WSACleanup();
                } 
				else
				{
                    // Receiving Message From User
                    std::string recvMsg = "";
                    recvMsg.assign(buf,bytesCount);
                    std::cout << "Message Received: " << recvMsg << std::endl;



                    std::string cUser = "";
                    if(userMap.find(socketList) != userMap.end())
                    {
                        cUser = userMap[socketList].userName;
                    }




                    bool checkTrue = true;
                    // Broadcast message to all clients
					for (int i = 0; i < readFd.fd_count; i++)
					{
						SOCKET broadSock = readFd.fd_array[i];

                        if(userList[i].socket == socketList)
                        {

                        }
                        // if socket is not its own, broadcast to all other sockets
						if (broadSock != serverSocket && broadSock != socketList)
						{

							std::stringstream ss;
							ss << "[" << cUser << "]: " << buf << "\r\n";
							std::string strOut = ss.str();

							send(broadSock, strOut.c_str(), strOut.size() + 1, 0);
                            //std::cout << "Sending" << std::endl;
                            checkTrue = true;

						}
                        else
                        {

                        }
					}



                    // Broadcast to single Client user


                }
            }
        }
    }


 

    /*

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

        */

        WSACleanup();
    

    return 0;


}