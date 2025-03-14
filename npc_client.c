#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ncurses.h>
#include "server_utils.h"
#include <poll.h>
#include <sys/time.h>

#define SERVER_PORT "4358"
#define QUIT 0

int main()
{
    srand(time(NULL));

    UserPacket map;

    struct pollfd pfds[2] = {
            {.fd = STDIN_FILENO, .events = POLLIN},
    };

    ncurs_setup();
    keypad(stdscr, TRUE);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    int s;
    if ((s = getaddrinfo("localhost", SERVER_PORT, &hints, &result)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    socklen_t length;
    struct addrinfo *rptr;
    for (rptr = result; rptr != NULL; rptr = rptr->ai_next)
    {
        sock_fd = socket(rptr->ai_family, rptr->ai_socktype, rptr->ai_protocol);
        if (sock_fd == -1)
            continue;

        if (connect(sock_fd, rptr->ai_addr, rptr->ai_addrlen) == -1)
        {
            if (close(sock_fd) == -1)
                perror("close");
            continue;
        }

        break;
    }

    if (rptr == NULL)
    {
        endwin(); //Not successful with any address
        fprintf(stderr, "Not able to connect\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result);

    enum types type = NPC;
    if (send(sock_fd, &type, sizeof(int), 0) == -1)
        perror("send");

    if (send(sock_fd, &type, sizeof(int), 0) == -1)
        perror("send");

    pid_t client_pid = getpid();
    if (send(sock_fd, &client_pid, sizeof(client_pid), 0) == -1)
        perror("send");

    if (recv(sock_fd, &map, sizeof(UserPacket), 0) == -1)
        perror("recv");

    if (strcmp(map.userMap, "Server full!") == 0)
    {
        printw("Server is full! Press any key to exit...");
        getch();
        endwin();
        return -1;
    }
    printMap(map, map.userMap);

    pfds[1].fd = sock_fd;
    pfds[1].events = POLLIN;
    int option;

    while (1)
    {
        option = makeMovePlayer(map);
        if (option == QUIT)
            break;

        int r = poll(pfds, 2, -1);
        if (r < 0)
        {
            perror("poll");
        }

        if (pfds[0].revents & POLLIN)
        {
            option = getch();
            if (option == 'q' || option == 'Q')
            {
                break;
            }
        }
        if (pfds[1].revents & POLLIN)
        {
            //Receive response from server
            if (recv(sock_fd, &map, sizeof(UserPacket), 0) == -1)
            {
                erase();
                printw("Connection with server lost! Press any key to exit...");
                getch();
                endwin();
                close(sock_fd);
                return ERROR_CONN;
            }
            printMap(map, map.userMap);

            memset(&map, 0, sizeof(map));
            if (send(sock_fd, &option, sizeof(int), MSG_NOSIGNAL) == -1)
            {
                erase();
                printw("Connection with server lost! Press any key to exit...");
                getch();
                endwin();
                close(sock_fd);
                return ERROR_CONN;
            }
        }
    }

    close(sock_fd);
    endwin();
    exit(EXIT_SUCCESS);
}