#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define MAX_USERS 10

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <string.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// GLOBAL VARIABLES
WSADATA wsaData;
SOCKET RecvSocket;

int usersNumber; // Number of users connected to server
int iResult;
int BufLen;
int SenderAddrSize;

struct sockaddr_in RecvAddr;
struct sockaddr_in SenderAddr;
struct sockaddr_in ConnectedUsers[MAX_USERS];

char SendBuf[1024];
char RecvBuf[1024];

unsigned short Port;


// method for initialization
int Initialize()
{
    iResult = 0;
    usersNumber = 0;
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

bool userAlreadyConnected(struct sockaddr_in currentUserAddress)
{
    int user;
    for (user = 0; user < usersNumber; user++)
    {
        if ((currentUserAddress.sin_addr.s_addr == ConnectedUsers[user].sin_addr.s_addr) 
            && (currentUserAddress.sin_port == ConnectedUsers[user].sin_port))
        {
            return 1;
        }
    }
    return 0;
}

void joinUser(struct sockaddr_in currentUserAddress)
{
    ConnectedUsers[usersNumber] = currentUserAddress;
    usersNumber++;
}

int sendResponse() {

    //---------------------------------------------
    // Send confirmation to client
    wprintf(L"Sending client confirmation for connecting to the server...\n");
    strcpy_s(SendBuf, "Connected to server!");

    wprintf(L"\nClient %d has connected to server!", usersNumber);
    wprintf(L"\nClient %d Address: %d", usersNumber, SenderAddr.sin_addr.s_addr);
    wprintf(L"\nClient %d Connected on Port: %d\n\n", usersNumber, SenderAddr.sin_port);

    iResult = sendto(RecvSocket,
        SendBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, sizeof(SenderAddr));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"Client not added with error: %d\n", WSAGetLastError());
        closesocket(RecvSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}


void receive()
{
    //-----------------------------------------------
    // Call the recvfrom function to receive datagrams
    // on the bound socket.
    wprintf(L"Receiving datagrams...\n");
    iResult = recvfrom(RecvSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) return;
        else wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
    }
    else {
        printf("-> Message received: %s \n", RecvBuf);
        if (!userAlreadyConnected(SenderAddr)) joinUser(SenderAddr);
        sendResponse();
    }

    strcpy_s(RecvBuf, ""); 
}

void Update()
{
    receive();    
}

int main()
{
    Initialize();

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


