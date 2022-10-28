#include "server_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked_list.h"
#include <unistd.h>
#include <math.h>


int numberOfPlayers = 0;
int numberOfBeasts = 0;
int colors[] = {PLAYER1_COL, PLAYER2_COL, PLAYER3_COL, PLAYER4_COL};

void prints(char *str){
    FILE *fptr = fopen("/dev/pts/7", "wr");
    fprintf(fptr,"%s", str);
    fclose(fptr);
}
void mapSnip(Player *player, Server *server){

    char temp[10];
    memset(player->packet.userMap, 0, USER_MAP_SIZE);
    //FILE *fptr = fopen("/dev/pts/6", "wr");

    player->packet.upper_x = player->packet.stats.pos_x - 2;
    player->packet.upper_y = player->packet.stats.pos_y - 2;

    int Y = player->packet.stats.pos_y - 2;
    for(int i = 0; i<5; i++){
        memset(temp, 0, 10);
        int X = player->packet.stats.pos_x - 2;
        for(int j = 0; j<5; j++){
            chtype chk = mvwinch(server->main_screen.window, Y, X);
            if(chk == ACS_CKBOARD)
                strcat(temp, "|");
            else if(X > MAP_HEIGHT-1 || X < 0 || Y > MAP_WIDTH-1 || Y < 0){
                strcat(temp, "|");
            }
            else {
                char ch = chk & A_CHARTEXT;
                strncat(temp, &ch,1);
            }
            X++;
        }
        strcat(temp, "\n");
        strcat(player->packet.userMap, temp);
        //prints(player->packet.userMap);
        Y++;
    }
}

int legalMoveChar(Server *server, int pos_x, int pos_y){
    char chk = server->map[pos_x][pos_y];
    if(chk != '+' && chk != '|' && chk != '-' && chk != '#'){
        return 1;
    }
    return 0;
}
int legalMove(WINDOW *p_win, int pos_x, int pos_y){
    chtype chk = mvwinch(p_win, pos_y, pos_x);
    if(chk != ACS_CKBOARD){
        return 1;
    }
    return 0;
}

int legalCoin(WINDOW *p_win, int pos_x, int pos_y){
    chtype chk = mvwinch(p_win, pos_y, pos_x);
    if(chk != ACS_CKBOARD && (chk & A_CHARTEXT) != 'o' && (chk & A_CHARTEXT) != '#'){
        return 1;
    }
    return 0;
}
int isInBush(WINDOW *p_win, int pos_x, int pos_y){
    chtype chk = mvwinch(p_win, pos_y, pos_x) & A_CHARTEXT;
    if(chk == '#'){
        return 1;
    }
    return 0;
}


void putCoin(Server *server, int pos_x, int pos_y, chtype type){
    mvwaddch(server->main_screen.window, pos_y, pos_x, type | COLOR_PAIR(TREASURE));
    wrefresh(server->main_screen.window);
}

void init_server_stats(Server *server){
    server->round_number = 0;
    server->dropped_treasures = ll_create();
    for(int i = 0; i<PLAYERS; i++){
        server->players[i].active = -1;
        server->players[i].packet.stats.type = PLAYER;
    }
    for(int i = 0; i<BEASTS; i++){
        server->beasts[i].active = -1;
        server->beasts[i].packet.stats.type = BEAST;
    }

}
int read_map(Server *server, char *filename){
    if(server == NULL || filename == NULL)
        return ERROR_PARAMS;

    FILE *Fptr = fopen(filename, "r");
    if(Fptr == NULL){
        perror(filename);
        return ERROR_FILE;
    }

    memset(server->map, 0, sizeof(server->map));
    int ch;
    for(int i = 0; i<MAP_WIDTH; i++){
        for(int j = 0; j<MAP_HEIGHT - 1; j++){
            server->map[i][j] = getc(Fptr);
        }
    }
    return ERROR_OK;

}

int init_main_screen(Server *server){
    if(server == NULL)
        return ERROR_PARAMS;

    server->main_screen.upper_x = 0;
    server->main_screen.upper_y = 0;

    server->main_screen.win_height = MAP_HEIGHT - 2;
    server->main_screen.win_width = MAP_WIDTH;

    server->main_screen.window = newwin(server->main_screen.win_width, server->main_screen.win_height, server->main_screen.upper_y, server->main_screen.upper_x);
    if(server->main_screen.window == NULL)
        return ERROR_WIN;

    box(server->main_screen.window, 0, 0);
    wrefresh(server->main_screen.window);
    return ERROR_OK;
}
int print_main_screen(Server *server) {
    for (int i = 0; i < MAP_WIDTH; i++) {
        for (int j = 0; j < MAP_HEIGHT - 1; j++) {
            char chk = mvwinch(server->main_screen.window, i, j) & A_CHARTEXT;
            if(chk == 'c' || chk == 'T' || chk == 't' || chk == 'D')
                continue;
            if (server->map[i][j] == '|' || server->map[i][j] == '-' || server->map[i][j] == '+')
                mvwaddch(server->main_screen.window, i, j, ACS_CKBOARD);
            else if(server->map[i][j] == '#'){
                mvwaddch(server->main_screen.window, i, j, server->map[i][j] | COLOR_PAIR(BUSH));
            }
            else if(server->map[i][j] == 'A'){
                //init_color(COLOR_YELLOW, 800,rand()%500+300, 0);
                mvwaddch(server->main_screen.window, i, j, server->map[i][j] | COLOR_PAIR(FIREPLACE));
            }
            else{
                mvwaddch(server->main_screen.window, i, j, server->map[i][j]);
            }
        }
    }
    wrefresh(server->main_screen.window);

    return ERROR_OK;
}
void erasePlayer(Player *player){
    player->active = -1;
    memset(&player->packet, 0, sizeof(UserPacket));
}
int init_stats_screen(Server *server){
    if(server == NULL)
        return ERROR_PARAMS;

    server->stats_screen.upper_x = 53;
    server->stats_screen.upper_y = 0;

    server->stats_screen.win_height = 70;
    server->stats_screen.win_width = 15;

    server->stats_screen.window = newwin(server->stats_screen.win_width, server->stats_screen.win_height, server->stats_screen.upper_y, server->stats_screen.upper_x);
    if(server->stats_screen.window == NULL)
        return ERROR_WIN;

    box(server->stats_screen.window, 0, 0);
    wrefresh(server->stats_screen.window);
    return ERROR_OK;
}
int init_err_panel(Server *server){
    if(server == NULL)
        return ERROR_PARAMS;

    server->err_panel.upper_x = 0;
    server->err_panel.upper_y = 25;

    server->err_panel.win_height = 60;
    server->err_panel.win_width = 3;

    server->err_panel.window = newwin(server->err_panel.win_width, server->err_panel.win_height, server->err_panel.upper_y, server->err_panel.upper_x);
    if(server->err_panel.window == NULL)
        return ERROR_WIN;

    //box(server->err_panel.window, 0, 0);
    wrefresh(server->err_panel.window);
    return ERROR_OK;
}
void print_err(Server *server,char *msg){
    werase(server->err_panel.window);
    mvwprintw(server->err_panel.window,1,1,msg);
    wrefresh(server->err_panel.window);
}
void print(int a, int b){
    FILE *fptr = fopen("/dev/pts/2", "wr");
    fprintf(fptr,"%d %d\n", a,b);
    fclose(fptr);
}
void printc(char c){
    FILE *fptr = fopen("/dev/pts/2", "wr");
    fprintf(fptr,"%c\n", c);
    fclose(fptr);
}
void print_stats_screen(Server *server){
    char temp[100];
    memset(temp, 0, 100);
    int pid = getpid();
    sprintf(temp, "%d", pid);

    mvwprintw(server->stats_screen.window,1,1, "Server's PID: ");
    mvwprintw(server->stats_screen.window,1,15, temp);

    mvwprintw(server->stats_screen.window,2,2, "Campsite X/Y: 23/11");
    mvwprintw(server->stats_screen.window,3,2, "Round number: ");

    sprintf(temp, "%lu", server->round_number);
    mvwprintw(server->stats_screen.window,3,17, temp);

    mvwprintw(server->stats_screen.window,5,1, "Parameter: ");
    mvwprintw(server->stats_screen.window,6,2, "PID ");
    mvwprintw(server->stats_screen.window,7,2, "TYPE ");
    mvwprintw(server->stats_screen.window,8,2, "Curr X/Y ");
    mvwprintw(server->stats_screen.window,9,2, "Deaths ");
    mvwprintw(server->stats_screen.window,11,2, "Coins ");
    mvwprintw(server->stats_screen.window,12,3, "carried ");
    mvwprintw(server->stats_screen.window,13,3, "brought");

    int start_x = 15;
    int start_y = 5;
    for(int i = 0; i<PLAYERS; i++){
        memset(temp, 0, 100);
        int active = server->players[i].active;
        if(active != 1){
            sprintf(temp, "%d", i+1);
            mvwprintw(server->stats_screen.window,start_y,start_x - 1, "       ");
            mvwprintw(server->stats_screen.window,start_y,start_x, "Player");
            mvwprintw(server->stats_screen.window,start_y,start_x+6, temp);
            mvwprintw(server->stats_screen.window,start_y + 1,start_x, "        ");
            mvwprintw(server->stats_screen.window,start_y+1,start_x, "-");
            mvwprintw(server->stats_screen.window,start_y + 2,start_x, "        ");
            mvwprintw(server->stats_screen.window,start_y+2,start_x, "-");
            mvwprintw(server->stats_screen.window,start_y+3,start_x, "--/--");
            mvwprintw(server->stats_screen.window,start_y+4,start_x, "- ");
        }
        else{
            sprintf(temp, "%d", i+1);
            mvwprintw(server->stats_screen.window,start_y+3,start_x+1, "     "); //clear
            mvwprintw(server->stats_screen.window,start_y+7,start_x, "     "); //clear

            mvwaddch(server->stats_screen.window,start_y,start_x-1, server->players[i].packet.usr | COLOR_PAIR(server->players[i].packet.stats.color));
            mvwprintw(server->stats_screen.window,start_y,start_x, "Player");
            mvwprintw(server->stats_screen.window,start_y,start_x+6, temp);
            mvwprintw(server->stats_screen.window,start_y+1,start_x, "       ");
            mvwprintw(server->stats_screen.window,start_y+1,start_x,"%d",server->players[i].user_pid); //TODO
            if(server->players[i].packet.stats.type == PLAYER){
                mvwprintw(server->stats_screen.window,start_y+2,start_x, "HUMAN");
            }
            else if(server->players[i].packet.stats.type == NPC){
                mvwprintw(server->stats_screen.window,start_y+2,start_x, "CPU");
            }
            sprintf(temp, "%d", server->players[i].packet.stats.pos_x);
            mvwprintw(server->stats_screen.window,start_y+3,start_x, temp);
            mvwprintw(server->stats_screen.window,start_y+3,start_x+2, "/");
            sprintf(temp, "%d", server->players[i].packet.stats.pos_y);
            mvwprintw(server->stats_screen.window,start_y+3,start_x+3, temp);
            sprintf(temp, "%d", server->players[i].packet.stats.deaths);
            mvwprintw(server->stats_screen.window,start_y+4,start_x, temp);
            sprintf(temp, "%d", server->players[i].packet.stats.carried_coins);
            mvwprintw(server->stats_screen.window,start_y+7,start_x, temp);
            sprintf(temp, "%d", server->players[i].packet.stats.brought_coins);
            mvwprintw(server->stats_screen.window,start_y+8,start_x, temp);
        }
        start_x+=10;
    }

    wrefresh(server->stats_screen.window);
}
void init_colors(){
    init_pair(BUSH, COLOR_GREEN, COLOR_BLACK);
    init_pair(FIREPLACE, COLOR_YELLOW, COLOR_BLACK);
    init_pair(USER, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(TREASURE, COLOR_YELLOW, COLOR_BLACK);
    init_color(COLOR_GREEN, 600, 900, 600);
    init_pair(PLAYER1_COL, COLOR_CYAN, COLOR_BLACK);
    init_pair(PLAYER2_COL, COLOR_RED, COLOR_BLACK);
    init_pair(PLAYER3_COL, COLOR_BLUE, COLOR_BLACK);
    init_pair(PLAYER4_COL, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(ENEMY, COLOR_RED, COLOR_BLACK);
}
void ncurs_setup(){
    initscr();
    curs_set(0);
    cbreak();
    noecho();
    start_color();
    init_colors();
}


void init_player(Server *server, enum types type, pid_t pid_client, int player_number){

        int x = rand()%(MAP_HEIGHT-3);
        int y = rand()%(MAP_WIDTH-2);
        while(legalMove(server->main_screen.window, x,y) != 1 ) {
            x = rand() % (MAP_HEIGHT - 3);
            y = rand() % (MAP_WIDTH - 2);
        }
        if(type == PLAYER || type == NPC) {
            server->players[player_number].packet.usr = 'o';
            server->players[player_number].packet.stats.spawn_x = x;
            server->players[player_number].packet.stats.spawn_y = y;

            server->players[player_number].packet.stats.pos_x = server->players[player_number].packet.stats.spawn_x;
            server->players[player_number].packet.stats.pos_y = server->players[player_number].packet.stats.spawn_y;

            server->players[player_number].packet.stats.brought_coins = 0;
            server->players[player_number].packet.stats.server_pid = getpid();
            server->players[player_number].packet.stats.campsite_x = -1;
            server->players[player_number].packet.stats.campsite_y = -1;
            server->players[player_number].packet.stats.deaths = 0;
            server->players[player_number].packet.stats.type = type;
            server->players[player_number].user_pid = pid_client;
            server->players[player_number].packet.stats.carried_coins = 0;
            server->players[player_number].packet.stats.color = colors[player_number];
            server->players[player_number].packet.stats.round_number = 0;
            server->players[player_number].packet.stats.player_number = player_number+1;
            server->players[player_number].active = 1;
            numberOfPlayers++;
        }
        else{
            server->beasts[player_number].packet.usr = '*';
            server->beasts[player_number].packet.stats.spawn_x = x;
            server->beasts[player_number].packet.stats.spawn_y = y;

            server->beasts[player_number].packet.stats.pos_x = server->beasts[player_number].packet.stats.spawn_x;
            server->beasts[player_number].packet.stats.pos_y = server->beasts[player_number].packet.stats.spawn_y;

            server->beasts[player_number].packet.stats.brought_coins = 0;
            server->beasts[player_number].packet.stats.server_pid = getpid();
            server->beasts[player_number].packet.stats.campsite_x = -1;
            server->beasts[player_number].packet.stats.campsite_y = -1;
            server->beasts[player_number].packet.stats.deaths = 0;
            server->beasts[player_number].packet.stats.type = type;
            server->beasts[player_number].user_pid = pid_client;
            server->beasts[player_number].packet.stats.carried_coins = 0;
            server->beasts[player_number].packet.stats.color = PLAYER2_COL;
            server->beasts[player_number].packet.stats.round_number = 0;
            server->beasts[player_number].packet.stats.player_number = player_number+1;
            server->beasts[player_number].active = 1;
        }




}

int isTreasure(Server *server, int pos_x, int pos_y){
    char chk = mvwinch(server->main_screen.window, pos_y, pos_x) & A_CHARTEXT;

    switch(chk) {
        case COIN:
            return COIN;
        case SMALL_TREASURE:
            return SMALL_TREASURE;
        case BIG_TREASURE:
            return BIG_TREASURE;
        case DROPPED_TREASURE:
            return DROPPED_TREASURE;
        default:
            return 0;
    }
}
void increaseCoins(Player *player, enum treasures treasure, int pos_x, int pos_y, linked_list *list){
    if(player->packet.stats.type == BEAST){

        return ;
    }
    int val = in_list(list, pos_x, pos_y);
    switch(treasure){
        case COIN:
            player->packet.stats.carried_coins += 1;
            break;
        case BIG_TREASURE:
            player->packet.stats.carried_coins += 50;
            break;
        case SMALL_TREASURE:
            player->packet.stats.carried_coins += 10;
            break;
        case DROPPED_TREASURE:
            player->packet.stats.carried_coins += val;
            break;
    }
}
Player *Collision(Server *server, Player *player){
    for(int i = 0; i<PLAYERS; i++){
        PlayerStats temp = server->players[i].packet.stats;
        if(temp.player_number == player->packet.stats.player_number)
            continue;
        if(temp.pos_x == player->packet.stats.pos_x && temp.pos_y == player->packet.stats.pos_y){
            return &server->players[i];
        }
    }
    for(int i = 0; i<BEASTS; i++){
        PlayerStats temp = server->beasts[i].packet.stats;
        if(temp.player_number == player->packet.stats.player_number)
            continue;
        if(temp.pos_x == player->packet.stats.pos_x && temp.pos_y == player->packet.stats.pos_y){
            return &server->beasts[i];
        }
    }
    return NULL;
}

void resetPlayer(Server *server, Player *player){
    player->packet.stats.pos_x = player->packet.stats.spawn_x;
    player->packet.stats.pos_y = player->packet.stats.spawn_y;
    player->packet.stats.carried_coins = 0;
    player->packet.stats.deaths += 1;

}
void setDroppedTreasure(Server *server, Player *player1, Player *player2){

    mvwaddch(server->main_screen.window, player1->packet.stats.pos_y, player1->packet.stats.pos_x, 'D' | COLOR_PAIR(TREASURE));
    ll_push_back(server->dropped_treasures, player1->packet.stats.pos_x, player1->packet.stats.pos_y, player1->packet.stats.carried_coins + player2->packet.stats.carried_coins);
    //push back linked list with player1 and player2 gold
    if(player1->packet.stats.type == BEAST && player2->packet.stats.type == BEAST){
        resetPlayer(server, player1);
        resetPlayer(server, player2);
    }
    if(player1->packet.stats.type != BEAST)
        resetPlayer(server, player1);
    if(player2->packet.stats.type != BEAST)
        resetPlayer(server, player2);

    print_main_screen(server);
    //set both players x,y to spawn, reset coins
}

int isCampsite(Server *server, int pos_x, int pos_y){
    char chk = mvwinch(server->main_screen.window, pos_y, pos_x) & A_CHARTEXT;
    if(chk == 'A')
        return 1;
    return 0;
}
void leaveTreasure(Player *player, int pos_x, int pos_y){
    player->packet.stats.brought_coins += player->packet.stats.carried_coins;
    player->packet.stats.carried_coins = 0;
    player->packet.stats.campsite_x = pos_x;
    player->packet.stats.campsite_y = pos_y;
}
void printPlayers(Server *server){
    for (int i = 0; i < PLAYERS; i++) {
        if(server->players[i].active == -1) continue;
        mvwaddch(server->main_screen.window, server->players[i].packet.stats.pos_y, server->players[i].packet.stats.pos_x, server->players[i].packet.usr | COLOR_PAIR(server->players[i].packet.stats.color));
    }
    for (int i = 0; i < BEASTS; i++) {
        if(server->beasts[i].active == -1) continue;
        mvwaddch(server->main_screen.window, server->beasts[i].packet.stats.pos_y, server->beasts[i].packet.stats.pos_x, server->beasts[i].packet.usr | COLOR_PAIR(server->beasts[i].packet.stats.color));
    }
}
void create_box(Server *server, bool flag, int move, Player *player)
{
    int x, y;
    x = 0;
    y = 0;

    if(flag == TRUE)
    {
        int ret;
        Player *temp_player;
        if (move == KEY_UP){
            if (legalMove(server->main_screen.window, player->packet.stats.pos_x, player->packet.stats.pos_y - 1)){

                if((ret = isTreasure(server, player->packet.stats.pos_x, player->packet.stats.pos_y - 1)) > 0)
                    increaseCoins(player, ret, player->packet.stats.pos_x, player->packet.stats.pos_y - 1, server->dropped_treasures);

                if(isCampsite(server, player->packet.stats.pos_x, player->packet.stats.pos_y - 1))
                    leaveTreasure(player, player->packet.stats.pos_x, player->packet.stats.pos_y - 1);

                if(player->inBush == 1){
                    player->inBush = 0;
                    player->packet.stats.pos_y++;
                }
                if(isInBush(server->main_screen.window, player->packet.stats.pos_x, player->packet.stats.pos_y - 1)){
                    player->inBush = 1;
                }

                if( (temp_player = Collision(server, player)) != NULL)
                    setDroppedTreasure(server, player, temp_player);
                else
                    player->packet.stats.pos_y--;
            }
        } else if (move == KEY_DOWN) {
            if (legalMove(server->main_screen.window, player->packet.stats.pos_x, player->packet.stats.pos_y + 1)){

                if((ret = isTreasure(server, player->packet.stats.pos_x, player->packet.stats.pos_y + 1)) > 0)
                    increaseCoins(player, ret, player->packet.stats.pos_x, player->packet.stats.pos_y + 1, server->dropped_treasures);

                if(isCampsite(server, player->packet.stats.pos_x, player->packet.stats.pos_y + 1))
                    leaveTreasure(player, player->packet.stats.pos_x, player->packet.stats.pos_y + 1);

                if(player->inBush == 1){
                    player->inBush = 0;
                    player->packet.stats.pos_y--;
                }
                if(isInBush(server->main_screen.window, player->packet.stats.pos_x, player->packet.stats.pos_y + 1)){
                    player->inBush = 1;
                }

                if( (temp_player = Collision(server, player)) != NULL)
                    setDroppedTreasure(server, player, temp_player);
                else
                    player->packet.stats.pos_y++;
            }
        } else if (move == KEY_LEFT) {
            if (legalMove(server->main_screen.window, player->packet.stats.pos_x - 1, player->packet.stats.pos_y)){

                if((ret = isTreasure(server, player->packet.stats.pos_x - 1, player->packet.stats.pos_y)) > 0)
                    increaseCoins(player, ret, player->packet.stats.pos_x - 1, player->packet.stats.pos_y, server->dropped_treasures);

                if(isCampsite(server, player->packet.stats.pos_x - 1, player->packet.stats.pos_y))
                    leaveTreasure(player,player->packet.stats.pos_x - 1, player->packet.stats.pos_y);

                if(player->inBush == 1){
                    player->inBush = 0;
                    player->packet.stats.pos_x++;
                }
                if(isInBush(server->main_screen.window, player->packet.stats.pos_x - 1, player->packet.stats.pos_y)){
                    player->inBush = 1;
                }

                if( (temp_player = Collision(server, player)) != NULL)
                    setDroppedTreasure(server, player, temp_player);
                else
                    player->packet.stats.pos_x--;
            }
        } else if (move == KEY_RIGHT) {
            if (legalMove(server->main_screen.window, player->packet.stats.pos_x + 1, player->packet.stats.pos_y)){

                if((ret = isTreasure(server, player->packet.stats.pos_x + 1, player->packet.stats.pos_y)) > 0)
                    increaseCoins(player, ret, player->packet.stats.pos_x + 1, player->packet.stats.pos_y, server->dropped_treasures);

                if(isCampsite(server, player->packet.stats.pos_x + 1, player->packet.stats.pos_y))
                    leaveTreasure(player, player->packet.stats.pos_x + 1, player->packet.stats.pos_y);

                if(player->inBush == 1){
                    player->inBush = 0;
                    player->packet.stats.pos_x--;
                }
                if(isInBush(server->main_screen.window, player->packet.stats.pos_x + 1, player->packet.stats.pos_y)){
                    player->inBush = 1;
                }

                if( (temp_player = Collision(server, player)) != NULL)
                    setDroppedTreasure(server, player, temp_player);
                else
                    player->packet.stats.pos_x++;

            }
        }

        print_main_screen(server);
        printPlayers(server);

    }
    //print(server->players[0].packet.stats.pos_x, server->players[0].packet.stats.pos_y);
    print(server->players[0].packet.stats.carried_coins, 0);

    wrefresh(server->main_screen.window);
    print_stats_screen(server);
}
void *serverInputThread(Server *server, int ch)
{

        int x = rand()%MAP_HEIGHT;
        int y = rand()%MAP_WIDTH;
        while(!legalCoin(server->main_screen.window, x,y)) {
            x = rand() % MAP_HEIGHT;
            y = rand() % MAP_WIDTH;
        }

        switch(ch)
        {
            case 'c':
                printc('c');
                putCoin(server, x,y,'c');
                break;
            case 'T':
                printc('T');
                putCoin(server, x,y,'T');
                break;
            case 't':
                printc('t');
                putCoin(server, x,y,'t');
                break;
        }

    generateLegend(server);
    create_box(server, TRUE, 0, NULL);
    return NULL;
}

void generateLegend(Server *server){
    server->legend_screen.window = newwin(11,60,15, 53);
    mvwprintw(server->legend_screen.window,1,1, "Legend: \n");
    for(int i = 0, k = 1; i<PLAYERS; i++,k++){
        mvwaddch(server->legend_screen.window,2,k, 'o' | COLOR_PAIR(colors[i]));
    }
    mvwprintw(server->legend_screen.window,2,5, " - players\n");
    mvwaddch(server->legend_screen.window,3,1, ACS_CKBOARD);
    mvwprintw(server->legend_screen.window,3,6, "- wall\n");
    mvwaddch(server->legend_screen.window,4,1, '*' | COLOR_PAIR(ENEMY));
    mvwprintw(server->legend_screen.window,4,2, "    - wild beast\n");

    mvwaddch(server->legend_screen.window, 5,1, 'c' | COLOR_PAIR(TREASURE));
    mvwprintw(server->legend_screen.window,5,2, "    - one coin                  ");
    mvwaddch(server->legend_screen.window,5,34, 'D' | COLOR_PAIR(TREASURE));
    mvwprintw(server->legend_screen.window, 5, 35, " - dropped treasure \n");
    mvwaddch(server->legend_screen.window,6,1, 't' | COLOR_PAIR(TREASURE));
    mvwprintw(server->legend_screen.window,6,2, "    - treasure (10 coins)\n");
    mvwaddch(server->legend_screen.window,7,1, 'T' | COLOR_PAIR(TREASURE));
    mvwprintw(server->legend_screen.window,7,2, "    - large treasure (50 coins)\n");
    mvwaddch(server->legend_screen.window,8,1, 'A' | COLOR_PAIR(FIREPLACE));
    mvwprintw(server->legend_screen.window,8,2, "    - campsite\n");


    wrefresh(server->legend_screen.window);
}
void generateLegendPlayer(){
    mvwprintw(stdscr,16,54, "Legend: \n");
    for(int i = 0, k = 54; i<PLAYERS; i++,k++){
        mvwaddch(stdscr,17,k, 'o' | COLOR_PAIR(colors[i]));
    }
    mvwprintw(stdscr,17,58, " - players\n");
    mvaddch(18,54, ACS_CKBOARD);
    mvwprintw(stdscr,18,59, "- wall\n");
    mvwaddch(stdscr, 19, 54, '*' | COLOR_PAIR(ENEMY));
    mvwprintw(stdscr,19,55, "    - wild beast\n");
    mvwaddch(stdscr, 20,54, 'c' | COLOR_PAIR(TREASURE));
    mvwprintw(stdscr,20,55, "    - one coin                  ");
    mvwaddch(stdscr,20,87, 'D' | COLOR_PAIR(TREASURE));
    mvwprintw(stdscr, 20, 88, " - dropped treasure \n");
    mvwaddch(stdscr,21,54, 't' | COLOR_PAIR(TREASURE));
    mvwprintw(stdscr,21,55, "    - treasure (10 coins)\n");
    mvwaddch(stdscr,22,54, 'T' | COLOR_PAIR(TREASURE));
    mvwprintw(stdscr,22,55, "    - large treasure (50 coins)\n");
    mvwaddch(stdscr,23,54, 'A' | COLOR_PAIR(FIREPLACE));
    mvwprintw(stdscr,23,55, "    - campsite\n");

    refresh();
}

void updateRounds(Server *server){
    for(int i = 0; i<PLAYERS; i++){
        server->players[i].packet.stats.round_number = server->round_number;
    }
    for(int i = 0; i<BEASTS; i++){
        server->beasts[i].packet.stats.round_number = server->round_number;
    }
}

void printMap(UserPacket packet, char *map_snippet){
    erase();
    int Y = packet.upper_y;
    int iter = 0;
    for(int i = 0; i<5; i++){
        int X = packet.upper_x;
        for(int j = 0; j<6; j++) {
            if (map_snippet[iter] == '|')
                mvaddch(Y,X,ACS_CKBOARD);
            else if(map_snippet[iter] == 'o' && iter == 14)
                mvaddch(Y,X, map_snippet[iter] | COLOR_PAIR(colors[packet.stats.player_number - 1]));
            else if(map_snippet[iter] == 'o')
                mvaddch(Y,X, map_snippet[iter] | COLOR_PAIR(ENEMY));
            else if(map_snippet[iter] == '#')
                mvaddch(Y,X, map_snippet[iter] | COLOR_PAIR(BUSH));
            else if(map_snippet[iter] == 'c' || map_snippet[iter] == 'T' || map_snippet[iter] == 't' || map_snippet[iter] == 'D')
                mvaddch(Y,X, map_snippet[iter] | COLOR_PAIR(TREASURE));
            else if(map_snippet[iter] == 'A')
                mvaddch(Y,X, map_snippet[iter] | COLOR_PAIR(FIREPLACE));
            else if(map_snippet[iter] == '*')
                mvaddch(Y,X,map_snippet[iter] | COLOR_PAIR(PLAYER2_COL));
            else
                mvaddch(Y,X,map_snippet[iter]);
            X++;
            iter++;
        }
        Y++;
    }
    printPlayerStats(packet);
    generateLegendPlayer();
    refresh();
}

void printPlayerStats(UserPacket packet){
    mvprintw(1,54, "Server's PID:  %d\n", packet.stats.server_pid);
    if(packet.stats.campsite_x == -1)
        mvprintw(2,54, " Campsite X/Y: unknown");
    else {
        mvprintw(2, 54, "                      ");
        mvprintw(2, 54, " Campsite X/Y: %d/%d\n", packet.stats.campsite_x, packet.stats.campsite_y);
    }
    mvprintw(3, 54, " Round number: %d\n", packet.stats.round_number);
    mvprintw(5, 54, "Player:\n");
    mvprintw(6, 54, " Number:       %d", packet.stats.player_number);
    if(packet.stats.type == NPC)
        mvprintw(7, 54, " Type:         CPU");
    else if(packet.stats.type == PLAYER)
        mvprintw(7, 54, " Type:         HUMAN");
    else
        mvprintw(7, 54, " Type:         BEAST");
    mvprintw(8, 54, "                        ");
    mvprintw(8, 54, " Curr X/Y:     %d/%d", packet.stats.pos_x, packet.stats.pos_y);
    mvprintw(9, 54, "                        ");
    mvprintw(9, 54, " Deaths:       %d", packet.stats.deaths);
    mvprintw(11, 54, "                        ");
    mvprintw(11, 54, " Coins found:       %d", packet.stats.carried_coins);
    mvprintw(12, 54, "                        ");
    mvprintw(12, 54, " Coins brought:     %d", packet.stats.brought_coins);
    refresh();
}
int random_move(){
    int res = rand()%4+1;
    if(res == 1)
        return KEY_UP;
    else if(res == 2)
        return KEY_DOWN;
    else if(res == 3)
        return KEY_LEFT;
    else
        return KEY_RIGHT;
}

int plot_line(int x0, int y0, int x1, int y1)
{
    int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    for (;;){
        if (x0 == x1 && y0 == y1) break;
        chtype ch1 = mvinch(y1,x1);
        chtype ch2 = mvinch(y0,x0);
        if(ch1 == ACS_CKBOARD || ch2 == ACS_CKBOARD) return -1;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
    return 0;
}

int distance(int x0, int y0, int x1, int y1){
    return sqrt(((x1-x0)*(x1-x0))+((y1-y0)*(y1-y0)));
}
int makeMoveBeast(UserPacket packet){
    int Y = packet.upper_y;
    int ret;
    for(int i = 0; i<5; i++){
        int X = packet.upper_x;
        for(int j = 0; j<5; j++) {
            if((mvinch(Y,X) & A_CHARTEXT) == 'o'){
                ret = plot_line(packet.stats.pos_x, packet.stats.pos_y, X,Y);
                if(ret == 0) {
                    int smallest = distance(Y + 1, X, packet.stats.pos_x, packet.stats.pos_y);
                    int move = KEY_DOWN;
                    if (distance(Y - 1, X, packet.stats.pos_x, packet.stats.pos_y) < smallest)
                        move = KEY_UP;
                    else if (distance(Y, X + 1, packet.stats.pos_x, packet.stats.pos_y) < smallest)
                        move = KEY_RIGHT;
                    else if (distance(Y, X - 1, packet.stats.pos_x, packet.stats.pos_y) < smallest)
                        move = KEY_LEFT;
                    return move;
                }
            }
        }
        Y++;
    }
    return random_move();
}
int makeMovePlayer(UserPacket packet){
    int Y = packet.upper_y;
    int ret;
    for(int i = 0; i<5; i++){
        int X = packet.upper_x;
        for(int j = 0; j<5; j++) {
            if((mvinch(Y,X) & A_CHARTEXT) == '*'){
                ret = plot_line(packet.stats.pos_x, packet.stats.pos_y, X,Y);
                if(ret == 0) {
                    int biggest = distance(Y + 1, X, packet.stats.pos_x, packet.stats.pos_y);
                    int move = KEY_DOWN;
                    if (distance(Y - 1, X, packet.stats.pos_x, packet.stats.pos_y) > biggest)
                        move = KEY_UP;
                    else if (distance(Y, X + 1, packet.stats.pos_x, packet.stats.pos_y) > biggest)
                        move = KEY_RIGHT;
                    else if (distance(Y, X - 1, packet.stats.pos_x, packet.stats.pos_y) > biggest)
                        move = KEY_LEFT;
                    return move;
                }
            }
        }
        Y++;
    }
    return random_move();
}