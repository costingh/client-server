#define _WINSOCK_DEPRECATED_NO_WARNINGS
 
#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define IP "127.0.0.1"

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <string.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")


WSADATA wsaData;
SOCKET SendSocket;

int iResult;
int BufLen;
int SenderAddrSize;

struct sockaddr_in SenderAddr;
sockaddr_in RecvAddr;

unsigned short Port;

char SendBuf[1024];
char RecvBuf[1024];

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

int sendTo()
{
    //---------------------------------------------
    // Send a datagram to the server
    strcpy_s(SendBuf, "Hello !!!!");
    wprintf(L"Sending message to server....\n");
    iResult = sendto(SendSocket,
        SendBuf, BufLen, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

void receiveConfirmation() {

    //-----------------------------------------------
    // Call the recvfrom function to receive server confirmation
    // on the bound socket.
    wprintf(L"Receiving datagrams...\n");
    iResult = recvfrom(SendSocket,
        RecvBuf, BufLen, 0, (SOCKADDR*)&SenderAddr, &SenderAddrSize);

    printf("%s \n", RecvBuf);
    strcpy_s(RecvBuf, "");


    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) return;
        else wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
    }    
}

void Update()
{ 
    receiveConfirmation();
}

int main()
{
    Initialize();
    sendTo();
    Sleep(1000); //sleeps 10 ms

    while (true)
    {
        Update();
        Sleep(1000); //sleeps 10 ms
    }
    //---------------------------------------------
    // When the application is finished sending, close the socket.
    wprintf(L"Finished sending. Closing socket.\n");
    iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Clean up and quit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    return 0;
}
