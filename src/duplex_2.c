#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    /* Create TCP socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET,
                  "127.0.0.1",
                  &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* Connect to server */
    if (connect(sock_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server!\n");
    printf("You can now chat.\n\n");

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        /* Monitor keyboard */
        FD_SET(STDIN_FILENO, &readfds);

        /* Monitor TCP socket */
        FD_SET(sock_fd, &readfds);

        int max_fd = sock_fd;

        /* Wait for keyboard OR network data */
        if (select(max_fd + 1,
                   &readfds,
                   NULL,
                   NULL,
                   NULL) < 0)
        {
            perror("select");
            break;
        }

        /* -------------------------------- */
        /* Check keyboard */
        /* -------------------------------- */

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
                break;

            send(sock_fd, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "exit", 4) == 0)
                break;
        }

        /* -------------------------------- */
        /* Check TCP socket */
        /* -------------------------------- */

        if (FD_ISSET(sock_fd, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            int bytes = recv(sock_fd,
                             buffer,
                             sizeof(buffer) - 1,
                             0);

            if (bytes <= 0)
            {
                printf("\nServer disconnected.\n");
                break;
            }

            buffer[bytes] = '\0';

            printf("\nServer: %s", buffer);
            printf("You: ");
            fflush(stdout);
        }
    }

    close(sock_fd);

    return 0;
}
