/**
 * The External Internet Address tool.
 * Prints your external IP address on the command line.
 *
 * By Set HM 2024.
 */
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <string.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#undef DEBUG

// Default HTTP request to be sent.
#define HTTP_REQUEST "GET /ip HTTP/1.1\r\n"\
                     "Host: ifconfig.me\r\n"\
                     "User-Agent: eip/1.0\r\n"\
                     "Accept: text/plain,*/*;q=0.8\r\n"\
                     "Accept-Language: en-US,es-MX;q=0.5\r\n"\
                     "Accept-Encoding: gzip,deflate,br\r\n"\
                     "Connection: close\r\n"\
                     "\r\n"
#define BUF_LEN 500

/**
 * Connects to host given its domain name and the target port number.
 */
SOCKET open_socket(const char* server_name, const char* port) {
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Translate the domain name into an IP address.
    if (getaddrinfo(server_name, port, &hints, &result) != 0) {
        printf("getaddrinfo() failed\n");
        WSACleanup();
        exit(-1);
    }

    // Attempt to connect to each returned IP address.
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sock == INVALID_SOCKET) {
            printf("socket() failed\n");
            WSACleanup();
            exit(-1);
        }

        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);
    return sock;
}

// Entry.
int main(void) {
    // Start WinSock2.
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup() failed\n");
        exit(-1);
    }

    // Connect to ifconfig.me via HTTP.
    SOCKET sock = open_socket("ifconfig.me", "80");

    if (sock == INVALID_SOCKET) {
        printf("Unable to connect to server\n");
        WSACleanup();
        exit(-1);
    }

    // Send the request.
    int bytes = send(sock, HTTP_REQUEST, (int)strlen(HTTP_REQUEST), 0);

    if (bytes == SOCKET_ERROR) {
        printf("send() failed\n");
        closesocket(sock);
        WSACleanup();
        exit(-1);
    }

#ifdef DEBUG
    printf("%d bytes sent.\n", bytes);
#endif

    // Receive the response.
    char buffer[BUF_LEN] = { 0 };

    do {
        bytes = recv(sock, buffer, BUF_LEN, 0);
#ifdef DEBUG
        printf("%d bytes read.\n", bytes);
#endif
        if (bytes < 0) {
            printf("recv() failed: %d\n");
        }
    } while (bytes > 0);

    // Show the results.
    puts(strstr(buffer, "\r\n\r\n") + 4);

    // Dipose all.
    closesocket(sock);
    WSACleanup();

    return 0;
}
