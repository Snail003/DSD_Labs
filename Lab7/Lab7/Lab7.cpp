// Lab7.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <windows.h>
#include <string>

struct Client1_Data
{
    int osVersion;
    int monitorAmount;
    int colorDepth;
};

struct Client2_Data
{
    int screenWidth;
    int iconWidth;
    int dpiHorizontal;
};

struct Data {
    uint8_t client;

    union {
        Client1_Data client1;
        Client2_Data client2;
    };
};

#define PIPE_NAME L"\\\\.\\pipe\\LOCAL"

void server()
{
    HANDLE hNamedPipe = CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, PIPE_UNLIMITED_INSTANCES, 512, 512, 5000, NULL);
    DWORD cbRead;

    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {
        fprintf(stdout, "CreateNamedPipe: Error %d \n", GetLastError());
        return;
    }

    printf("Pipe is created, waiting for connection...\n");

    while (true)
    {
        ConnectNamedPipe(hNamedPipe, NULL);

        printf("Client connected...\n");

        Data receivedData;

        bool result = ReadFile(hNamedPipe, &receivedData, sizeof(Data), (LPDWORD)&cbRead, NULL);
        if (result && (cbRead == sizeof(Data)))
        {
            if (receivedData.client == 0)
            {
                printf("Received data from client 1:\nOS version:%d\nMonitor amount:%d\nColour depth:%d\n",receivedData.client1.osVersion, receivedData.client1.monitorAmount, receivedData.client1.colorDepth);
            }
            else if (receivedData.client == 1)
            {
                printf("Received data from client 2:\nScreen width:%d\nIcon width:%d\nDPI horizontal:%d\n", receivedData.client2.screenWidth, receivedData.client2.iconWidth, receivedData.client2.dpiHorizontal);
            }
            else
            {
                printf("Invalid data (no such client data)\n");
            }
        }
        else
        {
            printf("Not enough received bytes or maybe some mistake.\n");
        }
        DisconnectNamedPipe(hNamedPipe);
        printf("Client disconnected...\n");
    }
    CloseHandle(hNamedPipe);
}

void client1()
{
    HANDLE hNamedPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {

        fprintf(stdout, "CreateFile: Error %d \n", GetLastError());
        return;
    }

    HDC hdc = GetDC(GetConsoleWindow());

    Data clientData;
    clientData.client = 0;
    clientData.client1.osVersion = GetSystemMetrics(SM_SERVERR2);
    clientData.client1.monitorAmount = GetSystemMetrics(SM_CMONITORS);
    clientData.client1.colorDepth = GetDeviceCaps(hdc, BITSPIXEL);

    printf("Sending data from client 1:\nOS version:%d\nMonitor amount:%d\nColour depth:%d\n", clientData.client1.osVersion, clientData.client1.monitorAmount, clientData.client1.colorDepth);

    if (WriteFile(hNamedPipe, &clientData, sizeof(Data), NULL, NULL))
    {
        printf("Data from client 1 was sent.\n");
    }
    else
    {
        printf("Failed to sent data from client 1.\n");
    }
}

void client2()
{
    HANDLE hNamedPipe = CreateFile(PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hNamedPipe == INVALID_HANDLE_VALUE)
    {

        fprintf(stdout, "CreateFile: Error %d \n", GetLastError());
        return;
    }

    HDC hdc = GetDC(GetConsoleWindow());

    Data clientData;
    clientData.client = 0;
    clientData.client2.screenWidth = GetSystemMetrics(SM_CXSCREEN);
    clientData.client2.iconWidth = GetSystemMetrics(SM_CXICON);
    clientData.client2.dpiHorizontal = GetDeviceCaps(hdc, LOGPIXELSX);

    printf("Sending data from client 2:\nScreen width:%d\nIcon width:%d\nDPI horizontal:%d\n", clientData.client2.screenWidth, clientData.client2.iconWidth, clientData.client2.dpiHorizontal);

    if (WriteFile(hNamedPipe, &clientData, sizeof(Data), NULL, NULL))
    {
        printf("Data from client 2 was sent.\n");
    }
    else
    {
        printf("Failed to sent data from client 2.\n");
    }
}

int main()
{
    printf("s - server\nc1 - client 1\nc2 - client 2\n");
    std::string input;

    std::getline(std::cin, input);

    if (input == "s")
    {
        server();
    }
    else if (input == "c1")
    {
        client1();
    }
    else if (input == "c2")
    {
        client2();
    }
    else
    {
        printf("Choose from listed ones.\n");
    }

    printf("Press Enter to close program...\n");
    getchar();
}