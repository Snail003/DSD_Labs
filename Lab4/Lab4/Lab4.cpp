#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "secret.h"
#include <iostream>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "6969"

struct Packet1 {
    uint8_t mode; // ALWAYS 1
    uint32_t ramSize;
    uint32_t diskAvailable;
    uint32_t screenWidth;
};

struct Packet2 {
    uint8_t mode; // ALWAYS 2
    uint32_t lineHeight;
    uint32_t notifWidth;
    uint32_t dpiHorizontal;
};

struct Packet3 {
    uint8_t mode; // ALWAYS 3
    uint64_t length;
    // After sending Packet3, we MUST send `length` bytes
};

int __cdecl main()
{

    // Prepare data to send
    struct Packet1 client1Packet;
    struct Packet2 client2Packet;
    struct Packet3 client3Packet;

    HDC hdc = GetDC(GetConsoleWindow());

    client1Packet.mode = 1;
    client1Packet.ramSize = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    client1Packet.diskAvailable = GetSystemMetrics(SM_REMOTECONTROL);
    client1Packet.screenWidth = GetDeviceCaps(hdc, HORZRES);
    
    printf("ramsize: %d\ndiskAvailable: %d\nscreenWidth: %d\n", GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_REMOTECONTROL), GetDeviceCaps(hdc, HORZRES));

    client2Packet.mode = 2;
    client2Packet.lineHeight = GetSystemMetrics(SM_CYEDGE);
    client2Packet.notifWidth = GetSystemMetrics(SM_CXEDGE);
    client2Packet.dpiHorizontal = GetDeviceCaps(hdc, LOGPIXELSX);

    printf("lineHeight: %d\nnotifWidth: %d\ndpiHorizontal: %d\n", GetSystemMetrics(SM_CYEDGE), GetSystemMetrics(SM_CXEDGE), GetDeviceCaps(hdc, LOGPIXELSX));

    client3Packet.mode = 3;
    const char *message = "amogus vs abobbbba";
    client3Packet.length = strlen(message) + 1;

    printf("\nChoose mode\n");
    int mode;
    
    std::cin >> mode;

    printf("\nMode: %d\n", mode);

    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL,
        * ptr = NULL,
        hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(DEFAULT_ADRESS, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL;ptr = ptr->ai_next) {

        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    if (mode == 1)
    {
        iResult = send(ConnectSocket, (const char*)&client1Packet, sizeof(client1Packet), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);
    }

    if (mode == 2)
    {
        iResult = send(ConnectSocket, (const char*)&client2Packet, sizeof(client2Packet), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);
    }

    if (mode == 3)
    {
        iResult = send(ConnectSocket, (const char*)&client3Packet, sizeof(client3Packet), 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);

        iResult = send(ConnectSocket, message, client3Packet.length, 0);
        if (iResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);
    }

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}