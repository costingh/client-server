#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <string.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#define IP "127.0.0.1"

int iResult;
WSADATA wsaData;
SOCKET SendSocket = INVALID_SOCKET;
sockaddr_in RecvAddr;
unsigned short Port = 27015;
char SendBuf[1024];
int BufLen = 1024;
u_long iMode = 0;

int Initialize()
{
    //----------------------
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

    iResult = ioctlsocket(SendSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);
    return 0;
}

int sendTo()
{
    strcpy_s(SendBuf, "Message Sent!");
    //---------------------------------------------
    // Send a datagram to the receiver
    wprintf(L"Sending a datagram to the receiver...\n");
    iResult = sendto(SendSocket,
        SendBuf, BufLen, 0, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult == SOCKET_ERROR) {
        wprintf(L"sendto failed with error: %d\n", WSAGetLastError());
        closesocket(SendSocket);
        WSACleanup();
        return 1;
    }
}

void Update()
{ 
    sendTo();
}

int main()
{
    Initialize();

    while (true)
    {
        Update();
        Sleep(2000); //sleeps 20 ms
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
