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


// DWORD - 32 bitlik Unsigned long deðeri WINAPI fonksiyonu olarak çaðýrýlmasý gerektiðinde
//LPVOID - long pointer to void -> void* handleClient((void* )arg)
DWORD WINAPI handleClient(LPVOID arg) {
    ClientInfo* client = (ClientInfo*)arg;
    SOCKET socket = client->socket;

    while (1) {
        //soketten okunan byteý dönderir
        int bytesReceived = recv(socket,client->buffer,BUFFER_SIZE,0);
        if (bytesReceived  > 0) {
            client-> buffer[bytesReceived] = '\0';
            printf("Received message: %s client fd: %d\n", client->buffer,socket);

            char* response = "Hello from server!";
            send(socket, response, sizeof(response), 0);
        }
        else {
            break;
        }
    }
    closesocket(socket);
    free(client);
    return NULL;
}

int main() {
    WSADATA wsaData;
  
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed");
        return 1;
    }

    // Server'ýn socketini oluþtur
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        printf("server_socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    // Server adres deðerlerini ver
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);

    //IF inet_pton == 0 -> IP geçersiz ancak iþlev baþarýlý
    if (inet_pton(AF_INET, "192.168.123.9", &serverAddress.sin_addr) < 0) {
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
        // Accept ile gelen isteði kabul et ve clientSocket oluþtur.
        //Client'ýn adresini bilmemize gerek olmadýðý için NULL parametreleri verildi
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket== INVALID_SOCKET) {
            printf("accept failed: %ld\n", WSAGetLastError());
            continue;
        }

        // kendisine ait buffer ve sockete sahip ClientInfo struct'ý oluþtur
        ClientInfo* clientInfo = (ClientInfo*)malloc(sizeof(ClientInfo));
        clientInfo->socket = clientSocket;

        //Thread ve Process iþlemlerinde HANDLE kullan.
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