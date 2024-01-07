
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define DESTINATION_ADDRESS "127.0.0.1"

int server_fd[4];

int64_t startserver(uint16_t port)
{
    // Create socket
    server_fd[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server_fd[0] < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    int socket_option = true;
    if (setsockopt(server_fd[0], SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    struct sockaddr_in server_address;
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd[0], (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    int result = listen(server_fd[0], 3);
    if (result < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    return result;
}

int64_t getmsg(char *buffer)
{
    // Accept a new connection
    int new_socket = accept(server_fd[0], NULL, NULL);
    if (new_socket < 0)
    {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Read the size of the incoming messages
    char message_buffer[MAX_BUFFER_SIZE] = {0};

    ssize_t valread  = read(new_socket, message_buffer, 5ULL);
    int message_size = atoi(message_buffer);
    // Read the complete message
    valread = read(new_socket, message_buffer, message_size);
    close(new_socket);

    // Copy the message to the provided buffer
    strcpy(buffer, message_buffer);

    return 0;
}

int64_t stop_server()
{
    close(server_fd[0]);
    return 0;
}

int64_t sndmsg(const char *message, uint16_t port)
{
    char message_buffer[1032] = {0};

    // Copy the message to the buffer
    strncpy(message_buffer, message, MAX_BUFFER_SIZE);
    message_buffer[MAX_BUFFER_SIZE] = '\0';

    // Create socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (socket_fd < 0)
    {
        puts("\nSocket creation error");
        return -1;
    }

    // Configure server address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port   = htons(port);

    // Convert IP address to binary form
    if (inet_pton(AF_INET, DESTINATION_ADDRESS, &server_address.sin_addr) <= 0)
    {
        puts("\nInvalid address/Address not supported");
        return -1;
    }

    // Connect to the server
    if (connect(socket_fd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0)
    {
        puts("\nConnection Failed");
        return -1;
    }

    const char leng[] = "1025";
    send(socket_fd, leng, sizeof(leng), 0);
    send(socket_fd, message_buffer, strlen(message_buffer), 0);

    // Close the socket
    close(socket_fd);
    return 0;
}
