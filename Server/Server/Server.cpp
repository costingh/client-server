#ifndef UNICODE
#define UNICODE
#endif
 
#define WIN32_LEAN_AND_MEAN
#define MAX_USERS 10
#define CHAR_LEN 1024

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <list>
#include <ctime>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define TIMEOUT_DELAY 15
#define PING_INTERVAL 1
#define QUALITY_INTERVAL 10

// GLOBAL VARIABLES
WSADATA wsaData;
SOCKET RecvSocket;

int usersNumber; // Number of users connected to server
int iResult;
int BufLen;
int SenderAddrSize;
int count;

struct sockaddr_in RecvAddr;
struct sockaddr_in SenderAddr;

char SendBuf[CHAR_LEN];
char RecvBuf[CHAR_LEN];
char serverResponseMessage[CHAR_LEN];
char message[CHAR_LEN-1];
char messageType[1];

unsigned short Port;

struct Client {
    struct sockaddr_in clientAddress;
    time_t lastPongTime, lastPingTime;
    double totalDelay;
    int pongMessages;
};

struct Message {
    char message[CHAR_LEN];
    struct sockaddr_in ClientAddress;
};


std::vector<struct Client> ConnectedClients;
std::list<struct Message> MessagesList;

enum actionType {
    CONNECT = 48,
    HELLO = 49,
    PING = 50,
};


// method for initialization
int Initialize()
{
    iResult = 0;
    usersNumber = 0;
    count = 0;
    Port = 27015;
    BufLen = 1024;
    SenderAddrSize = sizeof(SenderAddr);

    //-----------------------------------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error %d\n", iResult);
        return 1;
    }

    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (RecvSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    //-----------------------------------------------
    // Bind the socket to any address and the specified port.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //-----------------------------------------------
    // bind
    iResult = bind(RecvSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult != 0) {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return 1;
    }

    //-----------------------------------------------
    // set socket as non-blocking (set iMode = 1)
    u_long iMode = 1;
    iResult = ioctlsocket(RecvSocket, FIONBIO, &iMode);

    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);
    return 0;
}

int sendResponse(actionType action, int clientIndex) {
    iResult = 0;
    memset(SendBuf, 0, BufLen);

    switch (action)
    {
        case CONNECT: 
            SendBuf[0] = action;
            memcpy(SendBuf + 1, "User connected\n\0", strlen("User connected\n\0") + 1);
            wprintf(L"\nClient %d has connected to server!", usersNumber);
            wprintf(L"\nClient %d Address: %d", usersNumber, SenderAddr.sin_addr.s_addr);
            wprintf(L"\nClient %d Connected on Port: %d\n\n", usersNumber, SenderAddr.sin_port);
            break;
        
        case HELLO: 
            wprintf(L"Sending hello message confirmation...\n");
            SendBuf[0] = action;
            memcpy(SendBuf + 1, "Hello confirmation\n\0", strlen("Hello confirmation\n\0") + 1);
            break;
        
        case PING: 
            //wprintf(L"Send: PING\n");
            SendBuf[0] = action;
            memcpy(SendBuf + 1, "PING\n\0", strlen("PING\n\0") + 1);
            break;
        
        default:
            return -1;
    }   

    iResult = sendto(RecvSocket,
        SendBuf, strlen(SendBuf), 0, (SOCKADDR*)&ConnectedClients[clientIndex].clientAddress, sizeof(ConnectedClients[clientIndex].clientAddress));

    return iResult;
}

struct Client createClient(Message message) {
    struct Client client;
    client.clientAddress = message.ClientAddress;
    client.lastPongTime = time(NULL);
    client.lastPingTime = time(NULL);
    client.pongMessages = 0;
    client.totalDelay = 0;
    return client;
}

int parseMessage() {

    while (!MessagesList.empty()) {
        //  get the first message from message list
        struct Message message = MessagesList.front();

        // search in vector ConnectedClients for the client who sent the message, if it exists
        int user;
        for (user = 0; user < usersNumber; user++) {
            if (ConnectedClients[user].clientAddress.sin_port == message.ClientAddress.sin_port &&
                ConnectedClients[user].clientAddress.sin_addr.S_un.S_addr == message.ClientAddress.sin_addr.S_un.S_addr &&
                ConnectedClients[user].clientAddress.sin_family == message.ClientAddress.sin_family) {
                break;
            }
        }

        if (message.message[0] == CONNECT && user == usersNumber) {// if the user wants to connect and it s not already connected        
            struct Client client = createClient(message);// create a struct client        
            ConnectedClients.push_back(client);// add user to connected clients
            usersNumber++;

            iResult = sendResponse(CONNECT, usersNumber - 1);
            std::cout << "Server will start sending PING messages to client to keep connection alive..." << std::endl;
            if (!iResult) return iResult;
        }
        else if (message.message[0] == PING && user < usersNumber) {
            ConnectedClients[user].totalDelay += difftime(time(NULL), ConnectedClients[user].lastPongTime);// add the new delay
            ConnectedClients[user].lastPongTime = time(NULL);// reset lastPongTime
            ConnectedClients[user].pongMessages++;
        }

        // delete message from list after processing it
        MessagesList.pop_front();
    }
    return 0;
}

int receive()
{
    iResult = recvfrom(RecvSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);

    switch (RecvBuf[0])
    {
        case CONNECT: {
            strcpy_s(serverResponseMessage, "Confirm connection to server");
            break;
        }
        case HELLO: {
            strcpy_s(serverResponseMessage, "Confirm Hello Message");
            break;
        }
        case PING: {
            strcpy_s(serverResponseMessage, "Pong");
            break;
        }
        default: {
            strcpy_s(serverResponseMessage, "There was an error...");
            break;
        }
    }

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) return 0;
        else {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
            return 1;
        }
    }

    // create a new message and add it to list
    struct Message msg;
    msg.ClientAddress = SenderAddr;
    memcpy(msg.message, RecvBuf, BufLen);
    MessagesList.push_back(msg);

    return 0;
}

int Update()
{
    iResult = receive();    
    if (iResult != 0) return iResult;

    for (int user = 0; user < usersNumber; user++) {
        time_t current = time(NULL);

        // check if there was no PONG message sent for longer than 15s from client to server, if true, disconnect the client
        if (difftime(current, ConnectedClients[user].lastPongTime) > TIMEOUT_DELAY) {
            std::cout << "Client " << user << " has been dissconnected on port : " << ntohs(ConnectedClients[user].clientAddress.sin_port) << std::endl;
            ConnectedClients.erase(ConnectedClients.begin() + user);
            user--;
            usersNumber--;
            continue;
        }
        // set the server to send PING message once per second, if PING_INTERVAL = 1
        if (difftime(current, ConnectedClients[user].lastPingTime) >= PING_INTERVAL) {
            sendResponse(PING, user);
            ConnectedClients[user].lastPingTime = current;
        }
        if (count == QUALITY_INTERVAL) {
            std::cout << "##################################################" << std::endl;
            std::cout << "#                 Connection status               " << std::endl;
            std::cout << "#  Client connected through port: " << ntohs(ConnectedClients[user].clientAddress.sin_port) << std::endl;
            std::cout << "#  PONG messages received in last 10s: " << ConnectedClients[user].pongMessages << std::endl;

            // if the client has received pongMessages, then show the average delay
            if (ConnectedClients[user].pongMessages != 0) {
                std::cout << "#  Average delay: " << ConnectedClients[user].totalDelay / ((double)ConnectedClients[user].pongMessages) << "s" << std::endl;
            }
            std::cout << "##################################################" << std::endl << std::endl;

            // reset pongMessages and totalDelay
            ConnectedClients[user].pongMessages = 0;
            ConnectedClients[user].totalDelay = 0;
        }
    }
    // if count is bigger than 10s, reset it
    count++;
    if (count > QUALITY_INTERVAL) count = 0;

    iResult = parseMessage();
    return 0;
}


int main()
{
    Initialize();
    std::cout << "Server is running..." << std::endl;
    while (true)
    {
        Update();
        Sleep(1000); //sleeps 10 ms
    }
    
    //-----------------------------------------------
    // Close the socket when finished receiving datagrams
    wprintf(L"Finished receiving. Closing socket.\n");
    iResult = closesocket(RecvSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
        return 1;
    }

    //-----------------------------------------------
    // Clean up and exit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    return 0;
}


