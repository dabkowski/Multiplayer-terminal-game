#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <time.h>
#include <ncurses.h>
#include <pthread.h>
#include "linked_list.h"
#include "server_utils.h"
#include <fcntl.h>

/*  0 - listener
 *  1 - player - HUMAN/CPU
 *  2 - player - HUMAN/CPU
 *  3 - player - HUMAN/CPU
 *  4 - player - HUMAN/CPU
 *  5 - beast
 *  6 - beast
 *  7 - beast
 *  8 - beast

*/


/*
 * TODO
 * Add client/server error handling
 * Clean up messy code
 * Implement correct spawns
 * Player state reset after disconnection
 * PLayer limit
 * Beast limit
 * Beast spawning using separate process, exec()
 */


int main(void){
    srand(time(NULL));
    Server server;


    ncurs_setup();
    read_map(&server, "map.txt");
    init_server_stats(&server);
    init_main_screen(&server);
    init_err_panel(&server);
    keypad(server.main_screen.window, TRUE);

    generateLegend(&server);
    init_stats_screen(&server);
    print_stats_screen(&server);
    create_box(&server, TRUE, 0, NULL);


    //pthread_t server_in;
    //pthread_create(&server_in, NULL, serverInputThread, &server);

    //##########################################################


    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    // allow IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Stream socket
    hints.ai_flags = AI_PASSIVE;    // for wildcard IP address

    struct addrinfo *result;
    int s;
    if ((s = getaddrinfo(NULL, SERVER_PORT, &hints, &result)) != 0) {
        endwin();
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    // Scan through the list of address structures returned by
    //getaddrinfo. Stop when the the socket and bind calls are successful.

    int listener, optval = 1;
    struct addrinfo *rptr;
    for (rptr = result; rptr != NULL; rptr = rptr->ai_next) {
        listener = socket(rptr->ai_family, rptr->ai_socktype,
                          rptr->ai_protocol);
        if (listener == -1)
            continue;

        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
            print_err(&server, "Setsockopt error!");
        if (bind(listener, rptr->ai_addr, rptr->ai_addrlen) == 0)  // Success
            break;

        if (close(listener) == -1)
            print_err(&server, "Could not close listener!");
    }

    if (rptr == NULL) {               // Not successful with any address
        endwin();
        fprintf(stderr, "Not able to bind\n");
        exit(EXIT_FAILURE);
    }


    freeaddrinfo(result);

    // Mark socket for accepting incoming connections using accept
    if (listen(listener, BACKLOG) == -1)
        print_err(&server, "Listener error!");
    if(!fork()){
        prints("In fork");
        int fd = open("/dev/null", O_RDWR);
        dup2(fd, 0);
        dup2(fd, 1);
        dup2(fd, 2);
        execl("beast_client", "beast_client", "localhost", NULL);
    }

    struct pollfd pollfds_beasts[BEASTS];
    struct pollfd pollfds_sockets[3];
    struct pollfd pollfds_players[PLAYERS];

    int maxfds = 0, numfds = 0;
    nfds_t nfds = 0;
    maxfds = BEASTS;

    pollfds_sockets->fd = listener;
    pollfds_sockets->events = POLLIN;
    pollfds_sockets->revents = 0;

    (pollfds_sockets+1)->fd = STDIN_FILENO;
    (pollfds_sockets+1)->events = POLLIN;

    numfds = 0;

    for(int i = 0; i<PLAYERS; i++){
        pollfds_players[i].fd = -1;
    }
    for(int i = 0; i<BEASTS; i++){
        pollfds_beasts[i].fd = -1;
    }
    (pollfds_sockets+2)->fd = -1;
    socklen_t addrlen;
    struct sockaddr_storage client_saddr;


    while (1) {
        // monitor readfds for readiness for reading
        usleep(200000);
        nfds = numfds;
        if (poll(pollfds_sockets, 3, 0) == -1) // timer_fd
            perror("poll");
        if (poll(pollfds_beasts, nfds, 0) == -1) // timer_fd
            perror("poll");
        if (poll(pollfds_players, PLAYERS, 0) == -1) // timer_fd
            perror("poll");
        //Some sockets are ready. Examine readfds
        //check listening socket
        //loop through remaining sockets
        //if POLLIN, read and process data
        //else set as idle
        //send the data to all users
        //if((pollfds_sockets + 1)->revents & POLLIN){
        //
        //}
        if(((pollfds_sockets+1)->revents & POLLIN) == POLLIN){
            int option = wgetch(server.main_screen.window);

            if(option == 'Q' || option == 'q'){
                break;
            }
            if(option != 'b' && option != 'B'){
                serverInputThread(&server, option);
            }
            else if(option == 'b' || option == 'B'){
                if(((pollfds_sockets+2)->fd > 0)){
                    int action = COMM_SPAWN;
                    print_err(&server, "Sending command to beast listener!");
                    if(send((pollfds_sockets + 2)->fd, &action, sizeof(int), 0) == -1)
                        print_err(&server, "Error when sending a packet to beast listener!");
                }
            }
        }
        if( (pollfds_sockets->revents & POLLIN) == POLLIN && (pollfds_sockets->fd == listener)){
            addrlen = sizeof(struct sockaddr_storage);
            int fd_new;
            if ((fd_new = accept(listener, (struct sockaddr *) &client_saddr, &addrlen)) == -1)
                print_err(&server, "Connection could not be established!");
            // add fd_new to pollfds

            int who;
            ssize_t numbytes = recv(fd_new, &who, sizeof(int), 0);
            if(numbytes == -1){
                print_err(&server, "Error when receiving a packet!");
            }

            if(who == LISTENER){
                (pollfds_sockets + 2)->fd = fd_new;
                (pollfds_sockets + 2)->events = POLLIN;
                (pollfds_sockets + 2)->revents = 0;
                print_err(&server, "Beast listener has connected!");
                continue;
            }


            enum types type;
            numbytes = recv(fd_new, &type, sizeof(int), 0);
            if(numbytes == -1){
                print_err(&server, "Error when receiving a packet!");
            }

            if(type == BEAST) {
                if(numberOfBeasts == BEASTS){
                    print_err(&server, "Beast limit reached!");
                    close(fd_new);
                    continue;
                }
                for (int fd = 0; fd < BEASTS; fd++) {
                    if (((pollfds_beasts + fd)->fd < 0)) {
                        numfds++;
                        (pollfds_beasts + fd)->fd = fd_new;
                        (pollfds_beasts + fd)->events = POLLIN;
                        (pollfds_beasts + fd)->revents = 0;

                        pid_t pid_client;
                        numbytes = recv((pollfds_beasts + fd)->fd, &pid_client, sizeof(pid_t), 0);
                        if (numbytes == -1) {
                            print_err(&server, "Error when receiving a packet from beast!");
                        }

                        init_player(&server, type, pid_client, fd);
                        create_box(&server, TRUE, 0, NULL);
                        mapSnip(&server.beasts[fd], &server);
                        numberOfBeasts++;
                        if (send((pollfds_beasts + fd)->fd, &server.beasts[fd].packet, sizeof(UserPacket), 0) == -1)
                            print_err(&server, "Error when sending a to beast!");
                        print_err(&server, "Beast has been spawned!");
                        break;
                    }
                }
            }
            else if(type == PLAYER || type == NPC){
                if(numberOfPlayers == PLAYERS){
                    print_err(&server, "Server full!");
                    pid_t pid_client;
                    numbytes = recv(fd_new, &pid_client, sizeof(pid_t), 0);
                    if(numbytes == -1){
                        print_err(&server, "Error when receiving a packet from player!");
                    }
                    UserPacket err_msg = {.userMap="Server full!"};
                    if (send(fd_new, &err_msg, sizeof(UserPacket), 0) == -1)
                        print_err(&server, "Error when sending a packet to player!");
                    close(fd_new);
                    continue;
                }
                for(int fd = 0; fd < PLAYERS; fd++){
                    if(((pollfds_players + fd)->fd < 0)){

                        (pollfds_players + fd)->fd = fd_new;
                        (pollfds_players + fd)->events = POLLIN;
                        (pollfds_players + fd)->revents = 0;

                        pid_t pid_client;
                        numbytes = recv((pollfds_players + fd)->fd, &pid_client, sizeof(pid_t), 0);
                        if(numbytes == -1){
                            print_err(&server, "Error when receiving a packet from player!");
                        }

                        init_player(&server, type, pid_client, fd);
                        create_box(&server, TRUE, 0, NULL);
                        mapSnip(&server.players[fd], &server);
                        //numberOfPlayers++;
                        if (send((pollfds_players + fd)->fd, &server.players[fd].packet, sizeof(UserPacket), 0) == -1)
                            print_err(&server, "Error when sending a packet to player!");
                        print_err(&server, "Player has joined the game!");
                        break;
                    }
                }

            }

        }

        for(int fd = 0; fd < PLAYERS; fd++){
            if( (pollfds_players + fd)->revents & POLLIN){
                if((pollfds_players + fd)->fd <= 0 )
                    continue;

                int move;
                ssize_t numbytes = recv((pollfds_players + fd)->fd, &move, sizeof(int), 0);
                //print("Server: Message received");
                if (numbytes == -1)
                    perror("recv");
                else if (numbytes == 0) {
                    // connection closed by client
                    print_err(&server, "Connection closed by client! Player has left the game.");
                    //fprintf(stderr, "Socket %d closed by client\n", (pollfds_players + fd)->fd);
                    if (close((pollfds_players + fd)->fd) == -1)
                        print_err(&server, "Error when closing a connection with a player!");
                    erasePlayer(&server.players[fd]);
                    numberOfPlayers--;
                    (pollfds_players + fd)->fd *= -1; // make it negative so that it is ignored in future
                }
                // data from client
                create_box(&server, TRUE, move, &server.players[fd]);
                mapSnip(&server.players[fd], &server);
                int trash;
                while(recv((pollfds_players + fd)->fd, &trash, 1, MSG_DONTWAIT) > 0);

            }
            else{
                mapSnip(&server.players[fd], &server);
                continue;
                //IDLE
            }
        }
        for(int fd = 0; fd < BEASTS; fd++){
            if( (pollfds_beasts + fd)->revents & POLLIN){
                if((pollfds_beasts  + fd)->fd <= 0 )
                    continue;

                int move;
                ssize_t numbytes = recv((pollfds_beasts  + fd)->fd, &move, sizeof(int), 0);
                //print("Server: Message received");
                if (numbytes == -1)
                    print_err(&server, "Error when receiving a packet from beast!");
                else if (numbytes == 0) {
                    // connection closed by client
                    print_err(&server, "Connection closed by client! Beast has left the game!");
                    //fprintf(stderr, "Socket %d closed by client\n", (pollfds_beasts  + fd)->fd);
                    if (close((pollfds_beasts + fd)->fd) == -1)
                        print_err(&server, "Error when closing a connection with beast!");
                    numberOfBeasts--;
                    erasePlayer(&server.beasts[fd]);
                    (pollfds_beasts  + fd)->fd *= -1; // make it negative so that it is ignored in future
                }
                // data from client
                create_box(&server, TRUE, move, &server.beasts[fd]);
                mapSnip(&server.beasts[fd], &server);
                int trash;
                while(recv((pollfds_beasts + fd)->fd, &trash, 1, MSG_DONTWAIT) > 0);
            }
            else{
                mapSnip(&server.beasts[fd], &server);
                continue;
                //IDLE
            }
        }

        server.round_number++;
        updateRounds(&server);
        print_stats_screen(&server);

        for(int fd = 0; fd < PLAYERS; fd++){
            if ((pollfds_players + fd)->fd <= 0 || server.players[fd].active == -1)
                continue;

            if (send((pollfds_players + fd)->fd, &server.players[fd].packet, sizeof(UserPacket), 0) == -1)
                print_err(&server, "Error when sending a packet to the client!");
        }
        for(int fd = 0; fd < BEASTS; fd++){
            if ((pollfds_beasts + fd)->fd <= 0 || server.beasts[fd].active == -1)
                continue;

            if (send((pollfds_beasts + fd)->fd, &server.beasts[fd].packet, sizeof(UserPacket), 0) == -1)
                print_err(&server, "Error when sending a packet to the beast!");
        }

    }

    for(int i = 0; i<BEASTS; i++){
        if(pollfds_beasts[i].fd < 0) continue;
        close(pollfds_beasts[i].fd);
    }
    for(int i = 0; i<PLAYERS; i++){
        if(pollfds_players[i].fd < 0) continue;
        close(pollfds_players[i].fd);
    }
    for(int i = 0; i<3; i++){
        if(pollfds_sockets[i].fd < 0) continue;
        close(pollfds_sockets[i].fd);
    }


    endwin();
    return ERROR_OK;


}