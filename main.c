#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <windef.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 36
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024
#define IP "192.168.0.36"

CRITICAL_SECTION cs;

DWORD WINAPI HandleClient(LPVOID client_request_ptr) {
    char buffer[BUFFER_SIZE] = {0};
    char* hello = "Hello from client";
    
    int client_request = *((int*)client_request_ptr);
    free(client_request_ptr);

    SOCKET network_socket;
    int connection_status;

    if ((network_socket = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR) {
        printf("Socket creation failed\n");
        return 1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET,IP,&server_address.sin_addr) <= 0) {
        printf("Server address error\n");
        return 0;
    }

    if (connection_status = connect(network_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Error\n");
        return 0;
    }

    printf("Connection established\n");

    if (send(network_socket, (char*)hello, sizeof(hello), 0) < 0 ) {
        printf("Send failed");
        return 0;
    }
    if (recv(network_socket, buffer, 1024, 0) < 0)
    {
        printf("Receive failed\n");
    }
    printf("Client %d message: %s", network_socket, buffer);

    closesocket(network_socket);

    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET master_socket;
    struct sockaddr_in server_address;
    int client_request;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Create master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("socket failed\n");
        return 1;
    }

    // Initialize server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    if (inet_pton(AF_INET,IP,&server_address.sin_addr) <= 0) {
        printf("Invalid IP\n");
        return 1;
    }

    // Bind master socket
    if (bind(master_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("bind failed\n");
        return 1;
    }

    // Listen
    if (listen(master_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("listen failed\n");
        return 1;
    }

    printf("Waiting for connections...\n");

    while (1) {
        SOCKET new_socket;
        struct sockaddr_in client_address;
        int client_addrlen = sizeof(client_address);

        // Accept connection
        if ((new_socket = accept(master_socket, (struct sockaddr*)&client_address, &client_addrlen)) == INVALID_SOCKET) {
            printf("accept failed\n");
            return 1;
        }

        printf("New connection, socket fd is %d\n", new_socket);

        // Assuming client sends an integer request
        recv(new_socket, (char*)&client_request, sizeof(client_request), 0);

        // Create a thread for handling client request
        HANDLE thread_handle = CreateThread(NULL, 0, &HandleClient, (LPVOID)&client_request, 0, NULL);
        if (thread_handle == NULL) {
            printf("Failed to create thread\n");
            closesocket(new_socket);
        }
        else {
            printf("Thread created successfully\n");
            CloseHandle(thread_handle);
        }
    }

    closesocket(master_socket);
    WSACleanup();

    return 0;
}