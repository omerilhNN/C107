#include <windows.h>
#include <winsock2.h>
#include <stdio.h>

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
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create a socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Set up server address
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    if (inet_pton(AF_INET,"192.168.0.36",&serverAddress.sin_addr)) {
        printf("Invalid IP\n");
        return 1;
    }
   

    // Bind socket to address and port
    if (bind(listenSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        printf("bind failed: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    if (listen(listenSocket, 1) == SOCKET_ERROR) {
        printf("listen failed: %ld\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
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
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}