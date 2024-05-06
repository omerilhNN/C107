#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <windef.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 100
#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

CRITICAL_SECTION cs;

DWORD WINAPI HandleClient(LPVOID client_socket_ptr) {
    SOCKET client_socket = *((SOCKET*)client_socket_ptr);
    free(client_socket_ptr);
    char buffer[BUFFER_SIZE];

    int valread;
    if ((valread = recv(client_socket, buffer, BUFFER_SIZE, 0)) == SOCKET_ERROR) {
        // Baðlantýyý kapat
        closesocket(client_socket);
    }
    else if (valread == 0) {
        printf("Client disconnected\n");
        closesocket(client_socket);
    }
    else {
        buffer[valread] = '\0';
        printf("Received from client %d: %s\n", client_socket, buffer);

        // Serverdan gelen yanýt
        const char* response = "Hello from SERVER";
        send(client_socket, response, strlen(response), 0);
    }

    closesocket(client_socket);
    return NULL;
}

int main() {
    WSADATA wsa;
    SOCKET master_socket, new_socket;
    struct sockaddr_in server_address, client_address;
    int client_addrlen = sizeof(client_address);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("socket failed\n");
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET,"192.168.254.20",&server_address.sin_addr) <=0) {
        printf("Invalid Server address\n");
        return 1;
    }

    if (bind(master_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        printf("bind failed\n");
        return 1;
    }

    if (listen(master_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("listen failed\n");
        return 1;
    }

    printf("Waiting for connections...\n");

    InitializeCriticalSection(&cs);

    while (1) {
        if ((new_socket = accept(master_socket, (struct sockaddr*)&client_address, &client_addrlen)) == INVALID_SOCKET) {
            printf("accept failed\n");
            return 1;
        }
        printf("New connection, socket fd is %d\n", new_socket);

        SOCKET* client_socket_ptr = (SOCKET*)malloc(sizeof(SOCKET));
        *client_socket_ptr = new_socket;

        HANDLE thread_handle = CreateThread(NULL, 0, &HandleClient, client_socket_ptr, 0, NULL);
        if (thread_handle == NULL) {
            printf("Failed to create thread\n");
            closesocket(new_socket);
            free(client_socket_ptr);
        }
        else {
            printf("Thread created successfully\n");
            CloseHandle(thread_handle);
        }
    }

    DeleteCriticalSection(&cs);
    closesocket(master_socket);
    WSACleanup();

    return 0;
}
