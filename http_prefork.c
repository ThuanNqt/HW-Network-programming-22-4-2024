#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define MAX_CLIENTS 5
#define SERVER_PORT 7000
#define BUFFER_SIZE 1024
#define PROCESS_COUNT 8

int main()
{
    // create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1)
    {
        perror("Failed to create socket: ");
        exit(EXIT_FAILURE);
    }
    printf("Server socket created successfully\n");

    // server address struct
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(SERVER_PORT);

    // bind socket to address
    if (bind(server_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        perror("Failed to bind socket: ");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Socket binding successful\n");

    // set socket to listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        perror("Failed to listen on socket: ");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server is listening for connections...\n");

    // main loop to accept clients
    for (int i = 0; i < PROCESS_COUNT; i++)
    {
        if (fork() == 0) // child process
        {
            char client_buffer[BUFFER_SIZE];
            while (1)
            {
                int client_fd = accept(server_fd, NULL, NULL);
                int bytes_received = recv(client_fd, client_buffer, sizeof(client_buffer) - 1, 0);
                if (bytes_received <= 0)
                {
                    close(client_fd);
                    continue;
                }

                client_buffer[bytes_received] = '\0'; // null-terminate the string
                printf("Received: %s\n", client_buffer);

                char response[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
                                  "<html><body><h1>Welcome</h1><p>This is a test server.</p></body></html>";
                send(client_fd, response, strlen(response), 0);

                close(client_fd);
            }
            exit(EXIT_SUCCESS);
        }
    }

    getchar(); // wait for input to shut down server
    close(server_fd);
    kill(0, SIGTERM); // terminate the process group gracefully
    return EXIT_SUCCESS;
}