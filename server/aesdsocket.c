#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>

#define PORT 9000
#define START_LEN 128

int socket_fd, fd;
char *buffer = NULL;
char *sendBuf = NULL;
bool isConnectionInProgress = false;

// Close socket, file description, free buffer
void cleanup()
{
    if (socket_fd != -1)
    {
        close(socket_fd);
    }
    if (fd != -1)
    {
        close(fd);
    }
    if (buffer != NULL)
    {
        free(buffer);
    }
    if (sendBuf != NULL)
    {
        free(sendBuf);
    }
    remove("/var/tmp/aesdsocketdata");
    closelog();
}

// Signal handler
static void sigint_handler(int signo)
{
    while (isConnectionInProgress)
    {
    }
    syslog(LOG_DEBUG, "Caught signal, exiting");
    cleanup();
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int conn_fd, len, buflen, pos;
    int reuseaddr = 1;
    ssize_t len_recv, len_write, len_read;
    struct sockaddr_in server, client;
    char client_str[INET_ADDRSTRLEN];
    char *temp = NULL;
    off_t fsize;
    socklen_t client_size;
    bool useDaemon;
    pid_t pid;

    // If -d using Daemon (Create new child process)
    if (argc == 2)
    {
        if (strcmp(argv[1], "-d") == 0)
        {
            useDaemon = true;
        }
        else
        {
            printf("Command Line arg not recognized. Only -d can be used.\n");
            cleanup();
            exit(-1);
        }
    }
    else
    {
        useDaemon = false;
    }

    // Open log
    openlog("aesdsocket.c", 0, LOG_USER);

    // Register signal with OS - If fail write to log
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        perror("signal");
        printf("Couldn't handle SIGINT");
        cleanup();
        exit(-1);
    }
    if (signal(SIGTERM, sigint_handler) == SIG_ERR)
    {
        perror("signal");
        printf("Couldn't handle SIGTERM");
        cleanup();
        exit(-1);
    }

    // Socket creation (Create an endpoint for communication)
    // int ret = socket(domain, type, protocol)
    // Domain: PF_INET: IPv4; PF_INET6: IPv6
    // type: SOCK_STREAM (TCP - two way connected); UDP(Connectionless)
    // 0 - Choose proper for given type
    // int ret = socket(AF_INET, SOCK_STREAM, 0);
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        printf("Socket couldn't be created\n");
        cleanup();
        exit(-1);
    }

    // Set socket option
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(-1);
    }

    // Ensure the server variable is empty
    memset(&server, 0, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    // Bind the socket
    // Socket bind (Bind a name to a socket)
    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    // sockfd is fd from socket()
    // addr is address to bind
    // addrlen is the address length
    if ((bind(socket_fd, (struct sockaddr *)&server, sizeof(server))) < 0)
    {
        perror("bind");
        printf("Socket bind failed\n");
        cleanup();
        exit(-1);
    }

    // If use Deamon create a new process
    if (useDaemon)
    {
        pid = fork();
        if (pid == -1)
        {
            perror("fork");
            printf("Fork failed\n");
            cleanup();
            exit(-1);
        }
        else if (pid != 0)
        {
            cleanup();
            exit(EXIT_SUCCESS);
        }
    }

    //  listen: Listen for connections on socket
    //  int listen(int sockfd, int backlog)
    //  backlog is the number of pending connections allowed before refusing
    //  int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
    if ((listen(socket_fd, 10)) != 0)
    {
        perror("listen");
        printf("Listen failed\n");
        cleanup();
        exit(-1);
    }

    client_size = sizeof(client);

    // Open file for writing data
    fd = open("/var/tmp/aesdsocketdata", O_RDWR | O_CREAT | O_APPEND, 0644);
    while (1)
    {
        // Accept the socket
        conn_fd = accept(socket_fd, (struct sockaddr *)&client, &client_size);
        if (conn_fd == -1)
        {
            perror("accept");
            printf("Accept failed\n");
            cleanup();
            exit(-1);
        }

        isConnectionInProgress = true;

        inet_ntop(AF_INET, &(client.sin_addr), client_str, INET_ADDRSTRLEN);

        syslog(LOG_DEBUG, "Accepted connection from %s", client_str);

        buflen = START_LEN;
        pos = 0;

        // Malloc the buffer with the length
        buffer = (char *)malloc(sizeof(char) * buflen);
        if (buffer == NULL)
        {
            perror("malloc");
            printf("buffer malloc failed\n");
            cleanup();
            exit(-1);
        }

        // Ensure empty
        memset(buffer, 0, buflen);
        do
        {
            // Receive data from the socket
            len_recv = recv(conn_fd, buffer + pos, buflen - pos - 1, 0);
            if (len_recv == -1)
            {
                perror("recv");
                printf("Receive failed\n");
                cleanup();
                exit(-1);
            }
            else if (len_recv == buflen - pos - 1)
            {
                buflen += START_LEN;
                pos = buflen - START_LEN - 1;
                temp = (char *)realloc(buffer, buflen);
                if (temp == NULL)
                {
                    perror("realloc");
                    printf("Realloc failed\n");
                    cleanup();
                    exit(-1);
                }
                buffer = temp;
            }
            else
            {
                pos += len_recv;
            }
            *(buffer + (buflen - 1)) = 0;
        } while (strchr(buffer, '\n') == NULL); // Add new line

        len_write = write(fd, buffer, strlen(buffer));
        if (len_write == -1)
        {
            perror("write");
            printf("Write failed\n");
            cleanup();
            exit(-1);
        }
        else if (len_write != strlen(buffer))
        {
            perror("write");
            printf("Partial write occurred\n");
            cleanup();
            exit(-1);
        }

        if (fdatasync(fd) == -1)
        {
            perror("fdatasync");
            printf("fdatasync failed\n");
            cleanup();
            exit(-1);
        }

        fsize = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);

        len = fsize;

        sendBuf = (char *)malloc(sizeof(char) * (len + 1));
        if (sendBuf == NULL)
        {
            perror("malloc");
            printf("sendBuf malloc failed\n");
            cleanup();
            exit(-1);
        }

        temp = sendBuf;

        while (len != 0 && (len_read = read(fd, temp, len)) != 0)
        {
            if (len_read == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("read");
                printf("File read failure\n");
                cleanup();
                exit(-1);
            }
            len -= len_read;
            temp += len_read;
        }

        temp = sendBuf;
        len = fsize;

        while (len != 0 && (len_write = write(conn_fd, temp, len)) != 0)
        {
            if (len_write == -1)
            {
                if (errno == EINTR)
                    continue;
                perror("write");
                printf("Socket write failure\n");
                cleanup();
                exit(-1);
            }
            len -= len_write;
            temp += len_write;
        }

        syslog(LOG_DEBUG, "Closed connection from %s", client_str);

        free(sendBuf);
        sendBuf = NULL;
        free(buffer);
        buffer = NULL;
        close(conn_fd);
        isConnectionInProgress = false;
    }
}