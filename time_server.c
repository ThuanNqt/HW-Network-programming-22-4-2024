#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define SERVER_PORT 9000
#define BACKLOG 5
#define WORKER_PROCESSES 8
#define BUFFER_SIZE 256
#define TIME_COMMAND_LENGTH 19

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1) {
        perror("Failed to create socket\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        perror("Failed to bind socket\n");
        return EXIT_FAILURE;
    }

    if (listen(server_socket, BACKLOG)) {
        perror("Failed to listen on socket\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < WORKER_PROCESSES; i++) {
        if (fork() == 0) {
            char request_buffer[BUFFER_SIZE];
            char response_buffer[BUFFER_SIZE];

            while (1) {
                int client_socket = accept(server_socket, NULL, NULL);
                if (client_socket == -1) {
                    continue; // Skip to the next iteration if accept fails
                }
                printf("Client connected: %d\n", client_socket);

                int bytes_received = recv(client_socket, request_buffer, sizeof(request_buffer) - 1, 0);
                if (bytes_received <= 0) {
                    close(client_socket);
                    continue; // Skip to the next iteration if recv fails
                }
                request_buffer[bytes_received] = '\0'; // Null-terminate the request string
                printf("Received from client %d: %s\n", client_socket, request_buffer);

                time_t current_time;
                struct tm *time_info;
                time(&current_time);
                time_info = localtime(&current_time);
                // Adjust for Vietnam time (UTC+7)
                time_info->tm_hour += 7;

                // Normalize the hour if it goes beyond 24 and adjust day if necessary
                if (time_info->tm_hour >= 24) {
                    time_info->tm_hour -= 24;
                    time_info->tm_mday += 1;
                }

                // Handle different time commands
                if (strncmp(request_buffer, "GET_TIME dd/mm/yyyy", TIME_COMMAND_LENGTH) == 0) {
                    strftime(response_buffer, sizeof(response_buffer), "Date: %d/%m/%Y", time_info);
                } else if (strncmp(request_buffer, "GET_TIME dd/mm/yy", TIME_COMMAND_LENGTH - 2) == 0) {
                    strftime(response_buffer, sizeof(response_buffer), "Date: %d/%m/%y", time_info);
                } else {
                    snprintf(response_buffer, sizeof(response_buffer), "Invalid command");
                }

                send(client_socket, response_buffer, strlen(response_buffer), 0);
                close(client_socket);
            }
            exit(EXIT_SUCCESS);
        }
    }

    // Wait for input to terminate the server
    getchar();
    close(server_socket);
    kill(0, SIGTERM); // Use SIGTERM for a graceful shutdown
    return EXIT_SUCCESS;
}