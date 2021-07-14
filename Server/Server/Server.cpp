#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// GLOBAL VARIABLES
int iResult = 0;
WSADATA wsaData;
SOCKET RecvSocket;
struct sockaddr_in RecvAddr;
unsigned short Port = 27015;
char RecvBuf[1024];
int BufLen = 1024;
struct sockaddr_in SenderAddr;
int SenderAddrSize = sizeof(SenderAddr);
u_long iMode = 1;

// method for initialization
int Initialize()
{
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

    iResult = bind(RecvSocket, (SOCKADDR*)&RecvAddr, sizeof(RecvAddr));
    if (iResult != 0) {
        wprintf(L"bind failed with error %d\n", WSAGetLastError());
        return 1;
    }

    iResult = ioctlsocket(RecvSocket, FIONBIO, &iMode);
    if (iResult != NO_ERROR)
        printf("ioctlsocket failed with error: %ld\n", iResult);
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
    
    printf("%s \n", RecvBuf);

    if (iResult == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return;
        } 
        else {
            wprintf(L"recvfrom failed with error %d\n", WSAGetLastError());
        }
    }
   
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
        Sleep(2000); //sleeps 20 ms
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


