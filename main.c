#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <windef.h>

#pragma comment(lib, "ws2_32.lib")
#define PORT 36
#define BUFFER_SIZE 1024

// Structure to hold client information
typedef struct {
    SOCKET socket;
    char buffer[BUFFER_SIZE];
} ClientInfo;

// Function to handle client requests
DWORD WINAPI handleClient(LPVOID lpParameter) {
    ClientInfo* clientInfo = (ClientInfo*)lpParameter;
    SOCKET socket = clientInfo->socket;

    while (1) {
        int bytesReceived = recv(socket, clientInfo->buffer, BUFFER_SIZE, 0);
        if (bytesReceived > 0) {
            // Process client request
            printf("Received message from client: %s\n", clientInfo->buffer);

            // Send response back to client
            char* response = "Hello from server!";
            send(socket, response, strlen(response), 0);
        }
        else {
            break;
        }
    }

    closesocket(socket);
    free(clientInfo);
    return 0;
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    SOCKET listenSocket,clientSocket;
  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed");
        return 1;
    }

    // Server'ýn socketini oluþtur
    if ((listenSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) == INVALID_SOCKET) {
        printf("server_socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    // Server adres deðerlerini ver
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    //IF inet_pton == 0 -> IP geçersiz ancak iþlev baþarýlý
    if (inet_pton(AF_INET, "192.168.0.36", &serverAddress.sin_addr) < 0) {
        printf("Invalid IP\n");
        return 1;
    }


    // Listen Socketin adres ve porta atamasýný yap
    if (bind(listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("bind failed: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // listenSocket'i dinle
    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        printf("listen failed: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept ile gelen isteði kabul et clientSocket'e atamasýný yap.
        if ((clientSocket = accept(listenSocket, NULL, NULL) )== INVALID_SOCKET) {
            printf("accept failed: %ld\n", WSAGetLastError());
            continue;
        }

        // Create a new thread to handle client requests
        ClientInfo* clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->socket = clientSocket;

        HANDLE hThread = CreateThread(NULL, 0, handleClient, clientInfo, 0, NULL);
        if (hThread == NULL) {
            printf("CreateThread failed: %ld\n", GetLastError());
            closesocket(clientSocket);
            free(clientInfo);
            continue;
        }

        // Close the thread handle
        CloseHandle(hThread);
        free(clientInfo);
    }


    closesocket(listenSocket);
    WSACleanup();
    return 0;
}