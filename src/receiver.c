#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void send_file(int sockfd, const char *filepath)
{
    FILE *fp = fopen(filepath, "rb");
    if (!fp)
    {
        perror("fopen");
        return;
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    rewind(fp);

    const char *filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    char header[512];
    int header_len = snprintf(header, sizeof(header),
                               "FILE %s %ld\n", filename, filesize);

    send(sockfd, header, header_len, 0);

    char filebuf[BUFFER_SIZE];
    size_t n;
    long total_sent = 0;

    while ((n = fread(filebuf, 1, sizeof(filebuf), fp)) > 0)
    {
        size_t sent = 0;
        while (sent < n)
        {
            ssize_t s = send(sockfd, filebuf + sent, n - sent, 0);
            if (s <= 0)
            {
                perror("send");
                fclose(fp);
                return;
            }
            sent += (size_t)s;
        }
        total_sent += (long)sent;
    }

    fclose(fp);
    printf("Sent file '%s' (%ld bytes)\n", filename, total_sent);
}

void receive_file(int sockfd, char *buffer, int bytes)
{
    char header_copy[BUFFER_SIZE + 1];
    memcpy(header_copy, buffer, bytes);
    header_copy[bytes] = '\0';

    char *newline = strchr(header_copy, '\n');
    if (!newline)
    {
        printf("Malformed file header.\n");
        return;
    }
    *newline = '\0';

    char filename[256];
    long filesize;

    if (sscanf(header_copy, "FILE %255s %ld", filename, &filesize) != 2)
    {
        printf("Malformed file header.\n");
        return;
    }

    int header_text_len = (int)(newline - header_copy) + 1;
    int leftover = bytes - header_text_len;

    char outname[300];
    snprintf(outname, sizeof(outname), "received_%s", filename);

    FILE *fp = fopen(outname, "wb");
    if (!fp)
    {
        perror("fopen");
        return;
    }

    long total_received = 0;

    if (leftover > 0)
    {
        fwrite(buffer + header_text_len, 1, (size_t)leftover, fp);
        total_received += leftover;
    }

    char filebuf[BUFFER_SIZE];
    while (total_received < filesize)
    {
        long remaining = filesize - total_received;
        int to_read = remaining < BUFFER_SIZE ? (int)remaining : BUFFER_SIZE;

        int n = recv(sockfd, filebuf, to_read, 0);
        if (n <= 0)
        {
            printf("Connection lost during file transfer.\n");
            break;
        }

        fwrite(filebuf, 1, (size_t)n, fp);
        total_received += n;
    }

    fclose(fp);
    printf("\nReceived file saved as '%s' (%ld bytes)\n", outname, total_received);
    printf("You: ");
    fflush(stdout);
}

int main()
{
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    signal(SIGPIPE, SIG_IGN);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server!\n");
    printf("You can now chat. Use /sendimage <path> to send a file.\n\n");

    while (1)
    {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock_fd, &readfds);
        int max_fd = sock_fd;

        if (select(max_fd + 1, &readfds, NULL, NULL, NULL) < 0)
        {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            if (fgets(buffer, sizeof(buffer), stdin) == NULL)
                break;

            if (strncmp(buffer, "/sendimage ", 11) == 0)
            {
                char path[900];
                strncpy(path, buffer + 11, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
                path[strcspn(path, "\n")] = '\0';

                send_file(sock_fd, path);
                continue;
            }

            send(sock_fd, buffer, strlen(buffer), 0);

            if (strncmp(buffer, "exit", 4) == 0)
                break;
        }

        if (FD_ISSET(sock_fd, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));

            int bytes = recv(sock_fd, buffer, sizeof(buffer) - 1, 0);

            if (bytes <= 0)
            {
                printf("\nServer disconnected.\n");
                break;
            }

            if (bytes >= 5 && strncmp(buffer, "FILE ", 5) == 0)
            {
                receive_file(sock_fd, buffer, bytes);
            }
            else
            {
                buffer[bytes] = '\0';
                printf("\nServer: %s", buffer);
                printf("You: ");
                fflush(stdout);
            }
        }
    }

    close(sock_fd);

    return 0;
}
