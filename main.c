#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib") // Winsock Library
#define PORT 36
#define IP "192.168.0.36"
#define MAX_CLIENT 5


void* clientThread(void* arg)
{
    printf("In thread\n");
    char *message = "Hello from client\n";
    char buffer[1024];

    SOCKET clientSocket = *((SOCKET*)arg); 
    struct sockaddr_in server_address;
   
    WSADATA wsaClient;
    if (WSAStartup(MAKEWORD(2,2),&wsaClient)!=0) {
        printf("WSA Startup failed\n");
        pthread_exit(NULL);
    }
    //Client socket oluþtur
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        printf("Socket failed - CLIENT\n");
        pthread_exit(NULL);
    }
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    //IP atamasý yap
    if (inet_pton(AF_INET,"192.168.0.36",&server_address.sin_addr) < 0 ) {
        printf("Invalid IP\n");
        pthread_exit(NULL);
    }

    //Client soketi, bu server addresine baðlama iþlemini yap
    if (connect(clientSocket,(struct sockaddr*) &server_address,sizeof(server_address)) == SOCKET_ERROR) {
        printf("Connection to server failed\n");
        closesocket(clientSocket);
        pthread_exit(NULL);
    }
    *(SOCKET*)arg = clientSocket;

    ////send message from client to server
    //if (send(clientSocket, message, sizeof(message), 0) < 0)
    //{
    //    printf("Send failed\n");
    //}

    //// Receive message from server to buffer
    //if (recv(clientSocket, buffer, 1024, 0) < 0)
    //{
    //    printf("Receive failed\n");
    //}

    // Print the received message
    //printf("Data received: %s\n", buffer);

    WSACleanup();
    return arg;
}

int main()
{
    int i = 0;
    char buffer[1024];
    pthread_t tid[MAX_CLIENT];
    SOCKET serverSocket,clientSockets[MAX_CLIENT],newSocket; // Server socket tanýmlamasý
    struct sockaddr_in serverAddr;
    int serverAddrLen = sizeof(serverAddr);


    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Create the server socket.
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Socket creation failed.\n");
        return 1;
    }

    // Configure settings of the server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if (inet_pton(AF_INET,"192.168.0.36",&serverAddr.sin_addr) < 0 ) {
        printf("Invalid IP\n");
        return 1;
    }

    // Bind the server socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Bind failed.\n");
        return 1;
    }

    // Listen on the server socket
    if (listen(serverSocket, 1) == SOCKET_ERROR)
    {
        printf("Listen failed.\n");
        return 1;
    }

    // Accept incoming connections and handle them
    while (1)
    {
        for (int i = 0; i < MAX_CLIENT; i++) {
            if ((newSocket = accept(serverSocket, (struct sockaddr*)&serverAddr, &serverAddrLen)) == INVALID_SOCKET)
            {
                printf("Accept failed.\n");
                return 1;
            }
            clientSockets[i] = newSocket;

            if (pthread_create(&tid[i], NULL, &clientThread, (void*)&clientSockets[i]) != 0)
                printf("Failed to create thread\n");
        }
        for (int j = 0; j < MAX_CLIENT; j++) {
            if (pthread_join(tid[j],(void**)&clientSockets[j]) != 0) {
                printf("Failed to join thread\n");
            }
            if (recv(clientSockets[j],buffer,1024,0) < 0) {
                printf("Receive failed\n");
            }
            printf("Data %s for socket fd %d", buffer,clientSockets[j]);
        }
    }


    // Close the server socket
    closesocket(serverSocket);

    WSACleanup();

    return 0;
}
