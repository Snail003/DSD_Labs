// Lab6.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <windows.h>
#include <string>

struct Client1_Data
{
    int menuWidth;
    int hasMouse;
    int screenWidth;
};

struct Client2_Data
{
    int iconWidth;
    int iconHeight;
    int scrollWidth;
    int colorDepth;
};

struct Data {
    uint8_t isChanged;

    union {
        Client1_Data client1;
        Client2_Data client2;
    };
};

typedef Data TotalData[2];

#define CLIENT1_MUTEX L"hClient1Mutex"
#define CLIENT2_MUTEX L"hClient2Mutex"
#define MAP_FILE L"lab6_mapfile"

#define UPDATE_TIME 3000

void server()
{
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(TotalData), MAP_FILE);
    TotalData *totalData = (TotalData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TotalData));

    totalData[0]->isChanged = FALSE;
    totalData[1]->isChanged = FALSE;

    HANDLE hClient1Mutex = CreateMutex(NULL, FALSE, CLIENT1_MUTEX);
    if (hClient1Mutex == NULL) {
        std::cerr << "Couldn't create output mutex.\n";
        return;
    }

    HANDLE hClient2Mutex = CreateMutex(NULL, FALSE, CLIENT2_MUTEX);
    if (hClient1Mutex == NULL) {
        std::cerr << "Couldn't create output mutex.\n";
        return;
    }

    while (true)
    {
        Sleep(UPDATE_TIME);
        printf("\nUpdating data...\nClient 1:\n");
        WaitForSingleObject(hClient1Mutex, INFINITE);

        if (totalData[0]->isChanged)
        {
            printf("Menu width:%d\nHas mouse:%d\nScreen width:%d\n",totalData[0]->client1.menuWidth, totalData[0]->client1.hasMouse, totalData[0]->client1.screenWidth);
        }
        else
        {
            printf("Client 1 data hasn't changed\n");
        }
        totalData[0]->isChanged = FALSE;

        ReleaseMutex(hClient1Mutex);

        printf("Client 2:\n");
        WaitForSingleObject(hClient2Mutex, INFINITE);

        if (totalData[1]->isChanged)
        {
            printf("Icon width:%d\nIcon height:%d\nScroll width:%d\nColour depth:%d\n", totalData[1]->client2.iconWidth, totalData[1]->client2.iconHeight, totalData[1]->client2.scrollWidth, totalData[1]->client2.colorDepth);
        }
        else
        {
            printf("Client 2 data hasn't changed\n");
        }
        totalData[1]->isChanged = FALSE;

        ReleaseMutex(hClient2Mutex);
    }
    return;
}

void client1()
{
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, MAP_FILE);

    HANDLE hClientMutex = OpenMutex(SYNCHRONIZE, false, CLIENT1_MUTEX);

    TotalData *totalData = (TotalData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TotalData));

    HDC hdc = GetDC(GetConsoleWindow());

    WaitForSingleObject(hClientMutex, INFINITE);
    totalData[0]->client1.menuWidth = GetSystemMetrics(SM_CYMENU);
    totalData[0]->client1.hasMouse = GetSystemMetrics(SM_MOUSEPRESENT);
    totalData[0]->client1.screenWidth = GetDeviceCaps(hdc, HORZRES);
    totalData[0]->isChanged = TRUE;
    ReleaseMutex(hClientMutex);

    return;
}

void client2()
{
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, MAP_FILE);

    HANDLE hClientMutex = OpenMutex(SYNCHRONIZE, false, CLIENT2_MUTEX);

    TotalData* totalData = (TotalData*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TotalData));

    HDC hdc = GetDC(GetConsoleWindow());

    WaitForSingleObject(hClientMutex, INFINITE);
    totalData[1]->client2.iconWidth = GetSystemMetrics(SM_CXICON);
    totalData[1]->client2.iconHeight = GetSystemMetrics(SM_CYICON);
    totalData[1]->client2.scrollWidth = GetSystemMetrics(SM_CXVSCROLL);
    totalData[1]->client2.colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
    totalData[1]->isChanged = TRUE;
    ReleaseMutex(hClientMutex);

    return;
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
