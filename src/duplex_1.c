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
    int server_fd, client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    /* Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Configure server address */
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* Bind */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen */
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on 127.0.0.1:%d\n", PORT);
    printf("Waiting for client...\n");

    /* Accept client */
    client_fd = accept(server_fd, NULL, NULL);

    if (client_fd < 0)
    {
        perror("accept");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected!\n");
    printf("You can now chat.\n\n");

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        /* Monitor keyboard */
        FD_SET(STDIN_FILENO, &readfds);

        /* Monitor TCP socket */
        FD_SET(client_fd, &readfds);

        /* select() waits until one becomes ready */
        int max_fd = client_fd;

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
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

            send(client_fd, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "exit", 4) == 0)
                break;
        }

        /* -------------------------------- */
        /* Check TCP socket */
        /* -------------------------------- */

        if (FD_ISSET(client_fd, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            int bytes = recv(client_fd,
                             buffer,
                             sizeof(buffer) - 1,
                             0);

            if (bytes <= 0)
            {
                printf("\nClient disconnected.\n");
                break;
            }

            buffer[bytes] = '\0';

            printf("\nClient: %s", buffer);
            printf("You: ");
            fflush(stdout);
        }
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
