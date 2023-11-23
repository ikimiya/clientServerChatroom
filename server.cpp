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

bool compareString(std::string a, std::string b)
{
    if(a == b)
    {
        return true;
    }
    for(int i = 0; i < a.size(); i++)
    {
        if(toupper(a[i]) == toupper(b[i]))
        {
        }
        else
        {
            return false;
        }
    }
    return true;
}

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
        return -1;
    }
    else
    {
        std::cout << "Initialized Winsock:" << std::endl;
        std::cout << "Status: " << wsaData.szSystemStatus <<std::endl;
    }   // end wsastartup

    // create TCP socket 
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Failed: Error On Creating Socket" << std::endl;
        WSACleanup();
        return -1;
    }
    else
    {
        std::cout << "Ok: Socket Created" << std::endl;
    }   // end create tcp socket

    // connection of socket to port 5000
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
    }   // end bind
    
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
    }   // end listening


    // Create FD
    fd_set readFd;

    // clear socket from set
    FD_ZERO(&readFd);

    // add to set
    FD_SET(serverSocket,&readFd);

    bool keepGoing = true;

    while(keepGoing)
    {
        fd_set tempFD = readFd;

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
                    std::cout << "Failed: Error Receiving Ack from the Client: " << WSAGetLastError() << std::endl;

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
                    std::cout << "dupe user found !: " << std::endl;
                    std::stringstream dss;
                    std::string disconnect;
                    dss << "Username:" << account.userName << " has already been taken!\n"
                    << "Please Use a Different UserName! Good Bye!";

                    disconnect = dss.str();
                    send(newClient, disconnect.c_str(), disconnect.length(), 0);

                    closesocket(newClient);
                    FD_CLR(newClient, &readFd);
                }
                else
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
                }   // end check dupe user
            }
            else
            {   
                // else, if there is already user
				char buf[256];
                // windows set memory to zero
                ZeroMemory(buf,256);
                int bytesCount = recv(socketList, buf, sizeof(buf), 0);

                if (bytesCount <= 0) {
                    std::cout << "Failed: Error Receiving MSG from the Client: " << WSAGetLastError() << std::endl;

                    // delete if user DC
                    std::string cUser = "";
                    if(userMap.find(socketList) != userMap.end())
                    {
                        cUser = userMap[socketList].userName;
                    }
                    userMap.erase(socketList);
                    nameMap.erase(cUser);
					closesocket(socketList);
					FD_CLR(socketList, &readFd);
                    //WSACleanup();
                } 
				else
				{
                    // Find Current User
                    std::string cUser = "";
                    if(userMap.find(socketList) != userMap.end())
                    {
                        cUser = userMap[socketList].userName;
                    }

                    // Receiving Message From User
                    std::string recvMsg = "";
                    recvMsg.assign(buf,bytesCount);
                    std::cout << cUser<< " Message From Client: " << recvMsg << std::endl;

                    // ss
                    std::istringstream iss;
                    std::stringstream stringS;
                    std::string inputMsg = "";
                    std::string finalMsg = "";

                    iss.str("");
                    stringS.str("");
                    iss.clear();
                    stringS.clear();
                    
                    // put msg into stream
                    iss.str(recvMsg);
                    stringS << recvMsg;

                    inputMsg = stringS.str();
                    std::string temp = "";
                    std::string temp2 = "";

                    SOCKET sendToUser = SOCKET_ERROR;
                    std::string userString = "";

                    bool checkOnce = true;
                    bool directSend = false;

                    if(inputMsg[0] == '@')
                    {
                        while(std::getline(iss,temp,' '))
                        {
                            if(temp[0] == '@')
                            {
                                if(true)
                                {
                                    // without the @ + User
                                    temp2 = temp.erase(0,1);

                                    // look and check if exist
                                    if(nameMap.find(temp2) != nameMap.end())
                                    {
                                        // set send to socket
                                        sendToUser = nameMap[temp2].socket;
                                        userString = nameMap[temp2].userName;
                                    }
                                    else
                                    {
                                        //std::cout << "Name Not Found1" << std::endl;
                                    }
                                    
                                    // Compare First
                                    if(compareString(temp2,userString) )
                                    {
                                        std::cout << "Direct Message From:" <<cUser << " to " << userString << std::endl;
                                        directSend = true;
                                    }else
                                    {
                                        //std::cout << "Name Not Found2" << std::endl;
                                    }
                                    checkOnce = false;
                                }
                            }else
                            {
                                finalMsg = finalMsg + " " + temp;
                            }
                        }
                    }   // end checking for @

                    // Broadcast message to all clients
					for (int i = 0; i < readFd.fd_count; i++)
					{
						SOCKET broadSock = readFd.fd_array[i];

                        // check if direct message to single user
                        if(directSend)
                        {
                            if(broadSock != serverSocket && broadSock == sendToUser)
                            {
                                // sent to user
                                std::stringstream ss;
                                ss << "[PM:" << cUser << "]:" << finalMsg << "\r\n";
                                std::string strOut = ss.str();

                                send(broadSock, strOut.c_str(), strOut.size() + 1, 0);
                            }
                            else if(broadSock != serverSocket && broadSock == socketList)
                            {
                                // tell user that it is sent
                                std::stringstream ss;
                                ss << "[Message Sent to " << userString << "]"<< "\r\n";
                                std::string strOut = ss.str();
                                send(broadSock, strOut.c_str(), strOut.size() + 1, 0);
                            }
                        }
                        else if (broadSock != serverSocket && broadSock != socketList)
						{
                            // Broadcast to each user as public message
							std::stringstream ss;
							ss << "[" << cUser << "]: " << buf << "\r\n";
							std::string strOut = ss.str();
							send(broadSock, strOut.c_str(), strOut.size() + 1, 0);
						}
                        else
                        {
                        }   // end directsend
					}   // end broadcast for loop
                }   // end if else 
            }   // end else
        }   // end select for loop
    }   // end keepgoing

    closesocket(serverSocket);
    WSACleanup();

    return -1;
}