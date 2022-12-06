#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <chrono>
#include <cstdio>
#include <string>

#define BUFFER_LENGTH 512

int main(int argc, char **argv)
{
    WSADATA wsa_data;
    auto connect_socket = INVALID_SOCKET;
    struct addrinfo *addr_info = nullptr;
    struct addrinfo hints {};
    char buffer[BUFFER_LENGTH];
    int i_result;

    // Validate the parameters
    // IP address and port number are required (in that order)
    if (argc < 3) {
        printf("Error: IP address and port number are required (in that "
               "order)!\n");
        exit(-1);
    }

    uint32_t ping_pongs = 1000;

    if (argc == 4) {
        ping_pongs = std::atoi(argv[3]);
    }

    printf("Number of ping-pongs: %d\n", ping_pongs);

    // Initialize Winsock
    i_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (i_result != 0) {
        printf("WSAStartup failed with error: %d\n", i_result);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    i_result = getaddrinfo(argv[1], argv[2], &hints, &addr_info);
    if (i_result != 0) {
        printf("getaddrinfo failed with error: %d\n", i_result);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    connect_socket = socket(
        addr_info->ai_family, addr_info->ai_socktype, addr_info->ai_protocol);
    if (connect_socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Connect to server.
    i_result =
        connect(connect_socket, addr_info->ai_addr, (int)addr_info->ai_addrlen);
    if (i_result == SOCKET_ERROR) {
        closesocket(connect_socket);
        connect_socket = INVALID_SOCKET;
        printf("socket failed to connect: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addr_info);

    if (connect_socket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    auto stopwatch_begin = std::chrono::high_resolution_clock::now();

    int value = 0;
    auto value_str = std::to_string(value);
    memcpy(&buffer, &value_str[0], value_str.length());

    // Send an initial buffer
    i_result = send(connect_socket, buffer, BUFFER_LENGTH, 0);
    if (i_result == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(connect_socket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {
        i_result = recv(connect_socket, buffer, BUFFER_LENGTH, 0);
        if (i_result < 0) {
            printf("recv failed with error: %d\n", WSAGetLastError());
        } else if (i_result == 0) {
            printf("Connection closed\n");
        }

        value = std::atoi(buffer);
        if (value >= ping_pongs) {
            auto stopwatch_end = std::chrono::high_resolution_clock::now();
            auto microseconds =
                std::chrono::duration_cast<std::chrono::microseconds>(
                    stopwatch_end - stopwatch_begin)
                    .count();
            printf("Elapsed time: %.3f ms\n", (float)microseconds / 1000.0);
            break;
        }

        value++;
        auto value_str_tmp = std::to_string(value);

        memcpy(&buffer, &value_str_tmp[0], value_str_tmp.length());

        i_result = send(connect_socket, buffer, BUFFER_LENGTH, 0);
        if (i_result == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(connect_socket);
            WSACleanup();
            return 1;
        }
    } while (i_result > 0);

    // cleanup
    closesocket(connect_socket);
    WSACleanup();

    return 0;
}