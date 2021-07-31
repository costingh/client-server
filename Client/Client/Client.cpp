#define _WINSOCK_DEPRECATED_NO_WARNINGS
 
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define IP "127.0.0.1"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <ctime>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define TIMEOUT_DELAY 15

WSADATA wsaData;
SOCKET SendSocket;

struct sockaddr_in SenderAddr;
sockaddr_in RecvAddr;

int iResult;
int BufLen;
int SenderAddrSize;
int RecvAddrSize = sizeof(RecvAddr);

unsigned short Port;

char SendBuf[1024];
char RecvBuf[1024];
char message[1023];
char messageType[1];

time_t timeValue;

enum MESSAGE_TYPE {
    CONNECT = 48,
    HELLO = 49,
    PING = 50,
};

int Initialize()
{
    SendSocket = INVALID_SOCKET;
    Port = 27015;
    BufLen = 1024;
    SenderAddrSize = sizeof(SenderAddr);

    //---------------------------------------------
   // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        wprintf(L"WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    //---------------------------------------------
    // Create a socket for sending data
    SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SendSocket == INVALID_SOCKET) {
        wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    //---------------------------------------------
    // Set up the RecvAddr structure with the IP address of
    // the receiver (in this example case "192.168.1.1")
    // and the specified port number.
    RecvAddr.sin_family = AF_INET;
    RecvAddr.sin_port = htons(Port);
    RecvAddr.sin_addr.s_addr = inet_addr(IP);

    //-----------------------------------------------
    // set socket as non-blocking (set iMode = 1)
    u_long iMode = 1;
    iResult = ioctlsocket(SendSocket, FIONBIO, &iMode);

    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);
    return 0;
}

int sendTo(char type)
{
    int iResult = 0;
    memset(SendBuf, 0, BufLen);

    switch (type)
    {   
        case CONNECT: 
            wprintf(L"Connecting to server...\n");
            SendBuf[0] = type;
            memcpy(SendBuf + 1, "Connection request\n\0", strlen("Connection request\n\0") + 1);
            break;
        
        case HELLO: 
            wprintf(L"Sending Hello...\n");
            SendBuf[0] = type;
            memcpy(SendBuf + 1, "HELLO\n\0", strlen("HELLO\n\0"));
            break;
        
        case PING: 
            wprintf(L"Sending PONG...\n");
            SendBuf[0] = type;
            memcpy(SendBuf + 1, "PONG\n\0", strlen("PONG\n\0"));
            break;
        
        default:
            return -1;
    }
    
    iResult = sendto(SendSocket,
        SendBuf, strlen(SendBuf + 1) + 1, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }

    return 0;
}

int receive()
{
    memset(RecvBuf, 0, BufLen);
    iResult = recvfrom(SendSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&RecvAddr, &RecvAddrSize);

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) return 0;
        else {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
            return 1;
        }
    }

    switch (RecvBuf[0])
    {
        case CONNECT:
            time(&timeValue);
            // add 1 to hide first byte of RecvBuf which is the response type
            std::cout << RecvBuf + 1 << std::endl;
            break;

        case HELLO:
            wprintf(L"Receiving Hello...\n");
            break;

        case PING:
            if (difftime(time(NULL), timeValue) >= TIMEOUT_DELAY) {
                std::cout << "Connection timed out..." << std::endl;
                return -1;
            }
            time(&timeValue);
            std::cout <<"Receiving " << RecvBuf + 1;
            iResult = sendTo(PING);
            if (iResult != 0) return iResult;
            break;

        default:
            return -1;
    }

    return 0;
}

int connectToServer() {
    time(&timeValue);
    int iResult = sendTo(CONNECT);

    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        return iResult;
    }

    return 0;
}

int Update()
{     
    iResult = receive();
    if (iResult) return iResult;
    return 0;
}

int main()
{
    Initialize();

    iResult = connectToServer();
    if (iResult != 0) {
        wprintf(L"Server connection failed\n");
        iResult = closesocket(SendSocket);
        if (iResult == SOCKET_ERROR) {
            wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }
        WSACleanup();
        return 0;
    }

    while (true)
    {
        iResult = Update();
        if (iResult != 0) {
            wprintf(L"Update error\n");
            iResult = closesocket(SendSocket);
            if (iResult == SOCKET_ERROR) {
                wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
                WSACleanup();
                return 1;
            }
            WSACleanup();
            return 1;
        }
        Sleep(1000); //sleeps 10 ms
    }

    return 0;
}
