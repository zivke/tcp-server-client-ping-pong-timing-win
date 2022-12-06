#undef UNICODE

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstdio>
#include <string>

#define BUFFER_LENGTH 512

int main(int argc, char **argv)
{
    WSADATA wsa_data;
    int i_result;

    auto ListenSocket = INVALID_SOCKET;
    auto ClientSocket = INVALID_SOCKET;

    struct addrinfo *addr_info = nullptr;
    struct addrinfo hints {};

    int i_send_result;
    char buffer[BUFFER_LENGTH];

    // Initialize Winsock
    i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (i_result != 0) {
        printf("WSAStartup failed with error: %d\n", i_result);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (argc < 2) {
        printf("Error: IP address and port number are required (in that "
               "order)!\n");
        exit(-1);
    }

    // Resolve the server address and port
    i_result = getaddrinfo(nullptr, argv[1], &hints, &addr_info);
    if (i_result != 0) {
        printf("getaddrinfo failed with error: %d\n", i_result);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for the server to listen for client connections.
    ListenSocket = socket(
        addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(addr_info);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    i_result =
        bind(ListenSocket, addr_info->ai_addr, (int)addr_info->ai_addrlen);
    if (i_result == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(addr_info);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addr_info);

    i_result = listen(ListenSocket, SOMAXCONN);
    if (i_result == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // Accept a client socket
    ClientSocket = accept(ListenSocket, nullptr, nullptr);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    // No longer need server socket
    closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    do {
        i_result = recv(ClientSocket, buffer, BUFFER_LENGTH, 0);
        if (i_result > 0) {
            int value = std::atoi(buffer);
            value++;
            auto value_str_tmp = std::to_string(value);

            memcpy(&buffer, &value_str_tmp[0], value_str_tmp.length());

            // Echo the buffer back to the sender
            i_send_result = send(ClientSocket, buffer, i_result, 0);
            if (i_send_result == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
        } else if (i_result == 0) {
            printf("Connection closing...\n");
        } else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    } while (i_result > 0);

    // shutdown the connection since we're done
    i_result = shutdown(ClientSocket, SD_SEND);
    if (i_result == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}