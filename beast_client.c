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
#include <pthread.h>

#define SERVER_PORT "4358"
#define QUIT 0

void *beast_controller(void *arg){

    UserPacket map;
    struct pollfd pfds[2];

    char *name = (char *)arg;
    struct addrinfo hints;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    int s;
    if ((s = getaddrinfo (name, SERVER_PORT, &hints, &result)) != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        exit (EXIT_FAILURE);
    }

    /* Scan through the list of address structures returned by
       getaddrinfo. Stop when the the socket and connect calls are successful. */

    int sock_fd;
    socklen_t length;
    struct addrinfo *rptr;
    for (rptr = result; rptr != NULL; rptr = rptr -> ai_next) {
        sock_fd = socket (rptr -> ai_family, rptr -> ai_socktype,
                          rptr -> ai_protocol);
        if (sock_fd == -1)
            continue;

        if (connect (sock_fd, rptr -> ai_addr, rptr -> ai_addrlen) == -1) {
            if (close (sock_fd) == -1)
                perror("close");
            continue;
        }

        break;
    }

    if (rptr == NULL) {
        endwin();// Not successful with any address
        fprintf(stderr, "Not able to connect\n");
        exit (EXIT_FAILURE);
    }

    freeaddrinfo (result);

    enum types type = BEAST;
    if (send(sock_fd, &type, sizeof(int), 0) == -1)
        perror("send");
    if (send(sock_fd, &type, sizeof(int), 0) == -1)
        perror("send");
    pid_t client_pid = getpid();
    if (send(sock_fd, &client_pid, sizeof(client_pid), 0) == -1)
        perror("send");
    if (recv (sock_fd, &map, sizeof(UserPacket), 0) == -1)
        perror("recv");
    printMap(map, map.userMap);


    pfds[1].fd = sock_fd;
    pfds[1].events = POLLIN;
    int option = KEY_LEFT;
    if (send (sock_fd, &option, sizeof(int), MSG_NOSIGNAL) == -1)
        perror("send");

    while (1) {
        option = makeMoveBeast(map);
        if (option == QUIT)
            break;

        int r = poll(pfds, 2, -1);
        if (r < 0) { perror("poll"); }

        if (pfds[1].revents & POLLIN) {
            // receive response from server
            ssize_t ret;
            if ( (ret = recv (sock_fd, &map, sizeof(UserPacket), 0)) == -1 || ret == 0) {
                close(sock_fd);
                pthread_exit(NULL);
            }
            printMap(map, map.userMap);

            memset(&map, 0, sizeof(map));
            if (send (sock_fd, &option, sizeof(int), MSG_NOSIGNAL) == -1) {
                close(sock_fd);
                pthread_exit(NULL);
            }
        }
    }

    return NULL;
}

int main (int argc, char **argv)
{
    srand(time(NULL));

    struct pollfd pfds[2];

    ncurs_setup();
    keypad(stdscr, TRUE);
    
    if (argc != 2) {
        fprintf (stderr, "Usage: client hostname\n");
        exit (EXIT_FAILURE);
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof (struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *result;
    int s;
    if ((s = getaddrinfo (argv [1], SERVER_PORT, &hints, &result)) != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        exit (EXIT_FAILURE);
    }

    int sock_fd;
    struct addrinfo *rptr;
    for (rptr = result; rptr != NULL; rptr = rptr -> ai_next) {
        sock_fd = socket (rptr -> ai_family, rptr -> ai_socktype,
                          rptr -> ai_protocol);
        if (sock_fd == -1)
            continue;

        if (connect (sock_fd, rptr -> ai_addr, rptr -> ai_addrlen) == -1) {
            if (close (sock_fd) == -1)
                perror("close");
            continue;
        }

        break;
    }

    if (rptr == NULL) {               // Not successful with any address
        fprintf(stderr, "Not able to connect\n");
        exit (EXIT_FAILURE);
    }

    freeaddrinfo (result);

    enum types type = LISTENER;
    if (send(sock_fd, &type, sizeof(int), 0) == -1)
        perror("send");


    pfds[1].fd = sock_fd;
    pfds[1].events = POLLIN;
    pthread_t threads[BEASTS];
    while (1) {
        int msg;
        int r = poll(pfds, 2, -1);
        if (r < 0) { perror("poll"); }

        if (pfds[1].revents & POLLIN) {
            // receive response from server
            ssize_t ret;
            if ( (ret = recv (sock_fd, &msg, sizeof(int), 0)) == -1 || ret == 0){
                printw("Connection with server lost! Press any key to exit...");
                endwin();
                close(sock_fd);
                return ERROR_CONN;
            }
            if(msg == COMM_SPAWN){
                if(numberOfBeasts == BEASTS) continue;
                pthread_create(&threads[numberOfBeasts], NULL, beast_controller, argv[1]);
            }

        }
    }
}

