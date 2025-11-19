#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <iostream>
#include "secret.h"
#include <atomic>
#include <thread>
#include <windows.h>
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "7979"

typedef uint8_t PacketType;
#define PACKET_CLIENT_LOGIN 1
#define PACKET_CLIENT_MESSAGE_ALL 2
#define PACKET_CLIENT_LIST_USERS 3
#define PACKET_CLIENT_GET_USERNAME 4
#define PACKET_CLIENT_MESSAGE 5

#define PACKET_SERVER_LOGIN 101
#define PACKET_SERVER_MESSAGE 102
#define PACKET_SERVER_RECEIVE_MESSAGE 103
#define PACKET_SERVER_LIST_USERS 104
#define PACKET_SERVER_GET_USERNAME 105

typedef uint8_t ErrorType;
#define ERROR_SUCCESS 0
#define ERROR_UNKNOWN_USER_ID 1
#define ERROR_BAD_MESSAGE 2
#define ERROR_BAD_USERNAME 3

typedef uint64_t UserId;



struct PacketClient_Login {
    size_t usernameLength;
    // extra `usernameLength` bytes
};

struct PacketClient_MessageAll {
    size_t messageLength;
    // extra `messageLength` bytes
};

struct PacketClient_ListUsers {
};

struct PacketClient_GetUsername {
    UserId userId;
};

struct PacketClient_Message {
    UserId userId;
    size_t messageLength;
    // extra `messageLength` bytes
};



struct PacketServer_Login {
    ErrorType error;
    UserId userId;
};

struct PacketSercer_Message {
    ErrorType error;
};

struct PacketServer_ReceiveMessage {
    uint8_t isGlobalMessage;
    UserId userId;

    size_t messageLength;
    // extra `messageLength` bytes
};

struct PacketServer_ListUsers {
    uint64_t amount;
    // extra `amount * sizeof(UserId)` bytes
};

struct PacketServer_GetUsername {
    ErrorType error;
    size_t usernameLength;
    // extra `usernameLength` bytes
};

struct Header {
    PacketType type;
    union {
        PacketClient_Login client_login; // Ask server to assign you a user id
        PacketClient_MessageAll client_messageAll; // Broadcast a message to everyone
        PacketClient_ListUsers client_listUsers; // Get list of user ids connected to the server
        PacketClient_GetUsername client_getUsername; // Get username associated with a user id
        PacketClient_Message client_message; // Attempt to send a message to a specific user

        PacketServer_Login server_login; // RESPONSE to PacketClient_Login
        PacketSercer_Message server_message; // RESPONSE to PacketClient_Message or PacketClient_MessageAll
        PacketServer_ReceiveMessage server_receiveMessage; // Notify user of a new message
        PacketServer_ListUsers server_listUsers; // RESPONSE to PacketClient_ListUsers
        PacketServer_GetUsername server_getUsername; // RESPONSE to PacketClient_GetUsername
    };
};

std::atomic<bool> run(true);
UserId requestedUserId;

void ReadCin(std::atomic<bool>& run, SOCKET ConnectSocket)
{
    int iResult;
    std::string buffer;
    while (run.load())
    {
        std::string buffer;
        std::getline(std::cin, buffer);
        std::string words[2];
        std::string delimiter = " ";
        for (int i = 0; i < 2; i++) {
            words[i] = buffer.substr(0, buffer.find(delimiter));
            buffer.erase(0, buffer.find(" ") + delimiter.length());
        }

        if (buffer == "/ls")
        {
            struct Header header;
            header.type = PACKET_CLIENT_LIST_USERS;
            PacketClient_ListUsers list;
            header.client_listUsers = list;
            iResult = send(ConnectSocket, (const char*)&header, sizeof(header), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }
        }
        else if (words[0] == "/username")
        {
            struct Header header;
            header.type = PACKET_CLIENT_GET_USERNAME;
            PacketClient_GetUsername username;
            char* end;
            username.userId = strtoull(words[1].c_str(), &end,10);
            requestedUserId = username.userId;
            header.client_getUsername = username;
            iResult = send(ConnectSocket, (const char*)&header, sizeof(header), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }
        }
        else if (words[0] == "/global")
        {
            struct Header header;
            header.type = PACKET_CLIENT_MESSAGE_ALL;
            PacketClient_MessageAll globalMessage;
            std::string fullMessage = words[1] + " " + buffer;
            globalMessage.messageLength = fullMessage.size();
            header.client_messageAll = globalMessage;
            iResult = send(ConnectSocket, (const char*)&header, sizeof(header), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }

            iResult = send(ConnectSocket, fullMessage.c_str(), globalMessage.messageLength, 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }
        }
        else if (words[0] == "/whisper")
        {
            struct Header header;
            header.type = PACKET_CLIENT_MESSAGE;
            PacketClient_Message whisperMessage;
            char* end;
            whisperMessage.userId = strtoull(words[1].c_str(), &end, 10);
            whisperMessage.messageLength = buffer.size();
            header.client_message = whisperMessage;
            iResult = send(ConnectSocket, (const char*)&header, sizeof(header), 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }

            iResult = send(ConnectSocket, buffer.c_str(), whisperMessage.messageLength, 0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                run.store(false);
            }
        }
        else if (buffer == "/exit")
        {
            run.store(false);
        }
    }
}

int __cdecl main()
{
    // Initialize Winsock
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

    // Attempt to connect
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

    // Enter username

    std::string username;
    std::cout << "Enter username:\n";
    std::getline(std::cin, username);

    PacketClient_Login loginPacket;
    loginPacket.usernameLength = username.size();

    // Send login packet

    struct Header header;
    header.type = PACKET_CLIENT_LOGIN;
    header.client_login = loginPacket;

    iResult = send(ConnectSocket, (const char*)&header, sizeof(header), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult = send(ConnectSocket, username.c_str(), loginPacket.usernameLength, 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Wait for response

    iResult = recv(ConnectSocket,(char*)&header, sizeof(Header), 0);
    if (iResult < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
    }

    if (header.type != PACKET_SERVER_LOGIN) {
        printf("received wrong answer");
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    
    PacketServer_Login response = header.server_login;

    if (response.error != 0) {
        printf("Error %d", response.error);
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    UserId userId = response.userId;

    // printf("Logined with user ID: %lu\n", (unsigned long)userId);
    std::cout << "Logined with user ID: " << userId << std::endl;

    std::thread cinThread(ReadCin, std::ref(run), ConnectSocket);

    while (run.load())
    {
        struct Header header;
        iResult = recv(ConnectSocket, (char*)&header, sizeof(Header), 0);
        if (iResult < 0) {
            printf("recv failed with error: %d\n", WSAGetLastError());
        }
        if (iResult > 0) {
            switch (header.type) {
            case PACKET_SERVER_LOGIN: {
                printf("received wrong packet: %d\n", header.type);
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
            }break;
            case PACKET_SERVER_MESSAGE: {
                PacketSercer_Message messageVerification = header.server_message;
                if (messageVerification.error != 0) {
                    printf("Error %d", messageVerification.error);
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                printf("Successfully sent your message\n");
            }break;
            case PACKET_SERVER_RECEIVE_MESSAGE: {
                PacketServer_ReceiveMessage newMessage = header.server_receiveMessage;
                BOOL isGlobal = newMessage.isGlobalMessage;
                char* message = (char*)calloc(newMessage.messageLength + 1, sizeof(char));
                if (message == NULL) {
                    printf("Error during allocating memory");
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                iResult = recv(ConnectSocket, (char*)message, newMessage.messageLength, 0);
                struct Header newHeader;
                newHeader.type = PACKET_CLIENT_GET_USERNAME;
                PacketClient_GetUsername username;
                username.userId = newMessage.userId;

                newHeader.client_getUsername = username;
                iResult = send(ConnectSocket, (const char*)&newHeader, sizeof(newHeader), 0);
                if (iResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(ConnectSocket);
                    WSACleanup();
                    run.store(false);
                }

                iResult = recv(ConnectSocket, (char*)&header, sizeof(Header), 0);

                PacketServer_GetUsername usernameResponse = header.server_getUsername;
                if (usernameResponse.error != 0) {
                    printf("Error %d", usernameResponse.error);
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                char* receivedUsername = (char*)calloc(usernameResponse.usernameLength + 1, sizeof(char));
                if (receivedUsername == NULL) {
                    printf("Error during allocating memory");
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                iResult = recv(ConnectSocket, (char*)receivedUsername, usernameResponse.usernameLength, 0);
                if (newMessage.isGlobalMessage) {
                    std::cout << newMessage.userId << " - " << receivedUsername << ": " << message << std::endl;
                } else {
                    std::cout << "(Whisper) " << newMessage.userId << " - " << receivedUsername << ": " << message << std::endl;
                }
            }break;
            case PACKET_SERVER_LIST_USERS: {
                PacketServer_ListUsers users = header.server_listUsers;
                uint64_t amount = users.amount;
                UserId* userIds = (UserId*)calloc(amount, sizeof(UserId));
                if (userIds == NULL) {
                    printf("Error during allocating memory");
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                iResult = recv(ConnectSocket, (char*)userIds, amount * sizeof(UserId), 0);
                printf("Connected gamers:\n");
                for (int i = 0; i < amount; i++) {
                    std::cout << userIds[i] << std::endl;
                }
                       
            }break;
            case PACKET_SERVER_GET_USERNAME: {
                PacketServer_GetUsername username = header.server_getUsername;
                if (username.error != 0) {
                    printf("Error %d", username.error);
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                char* receivedUsername = (char*)calloc(username.usernameLength + 1, sizeof(char));
                if (receivedUsername == NULL) {
                    printf("Error during allocating memory");
                    closesocket(ConnectSocket);
                    WSACleanup();
                    return 1;
                }
                iResult = recv(ConnectSocket, (char*)receivedUsername, username.usernameLength, 0);
                std::cout << requestedUserId << " - " << receivedUsername << std::endl;
            }break;
            }
        }
    }

    run.store(false);
    cinThread.join();

    
    // Cleanup

    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}