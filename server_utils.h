#ifndef SECOND_TRY_SERVER_UTILS_H
#define SECOND_TRY_SERVER_UTILS_H

#include <ncurses.h>
#include "linked_list.h"
#include <unistd.h>

#define MAP_HEIGHT 53
#define MAP_WIDTH 25
#define USER_MAP_SIZE 35
#define PLAYERS 4
#define NPCS 4
#define BEASTS 4
#define SERVER_PORT "4358"
#define NUM_FDS 6
#define BACKLOG 10



extern int colors[];
extern int numberOfPlayers;
extern int numberOfBeasts;


enum player_colors{
    PLAYER1_COL = 115,
    PLAYER2_COL = 116,
    PLAYER3_COL = 117,
    PLAYER4_COL = 118
};

enum types{
    NPC = 0,
    PLAYER = 1,
    BEAST = 2,
    LISTENER = 3
};

enum commands{
    COMM_SPAWN = 'b',
};

enum errors{
    ERROR_PARAMS = -1,
    ERROR_FILE = -2,
    ERROR_OK = 0,
    ERROR_WIN = -5,
    ERROR_FULL = -3,
    ERROR_CONN = -4,
};

enum colors{
    BUSH = 1,
    FIREPLACE = 2,
    USER = 3,
    TREASURE = 4,
    ENEMY = 5,
};
enum treasures{
    SMALL_TREASURE = 't',
    COIN = 'c',
    BIG_TREASURE = 'T',
    DROPPED_TREASURE = 'D',
};


typedef struct PlayerStats{
    int carried_coins;
    int brought_coins;
    int pos_x;
    int pos_y;
    int spawn_x;
    int spawn_y;
    int player_number;
    enum types type;
    int campsite_x;
    int campsite_y;
    int round_number;
    int server_pid;
    int deaths;
    int color;

}PlayerStats;

typedef struct UserPacket{
    int upper_x;
    int upper_y;
    chtype usr;
    PlayerStats stats;
    char userMap[USER_MAP_SIZE];
}UserPacket;



typedef struct Player{
    UserPacket packet;
    int user_pid;
    int inBush;
    int active;
}Player;

typedef struct Win{
    int upper_x;
    int upper_y;
    WINDOW *window;
    int win_width;
    int win_height;
}Win;

typedef struct Server{
    char map[MAP_WIDTH][MAP_HEIGHT];
    Player players[PLAYERS];
    Player beasts[BEASTS];
    Win main_screen;
    Win stats_screen;
    Win legend_screen;
    Win err_panel;
    uint64_t round_number;
    linked_list *dropped_treasures;
}Server;


void prints(char *str);
void mapSnip(Player *player, Server *server);
int legalMoveChar(Server *server, int pos_x, int pos_y);
int legalMove(WINDOW *p_win, int pos_x, int pos_y);
int legalCoin(WINDOW *p_win, int pos_x, int pos_y);
int isInBush(WINDOW *p_win, int pos_x, int pos_y);
void putCoin(Server *server, int pos_x, int pos_y, chtype type);
void init_server_stats(Server *server);
int read_map(Server *server, char *filename);
int init_main_screen(Server *server);
int print_main_screen(Server *server);
int init_stats_screen(Server *server);
void print(int a, int b);
void printc(char c);
void print_stats_screen(Server *server);
void init_colors();
void ncurs_setup();
void init_player(Server *server, enum types type, pid_t pid_client, int player_number);
int isTreasure(Server *server, int pos_x, int pos_y);
void increaseCoins(Player *player, enum treasures treasure, int pos_x, int pos_y, linked_list *list);
Player *Collision(Server *server, Player *player);
void resetPlayer(Server *server, Player *player);
void setDroppedTreasure(Server *server, Player *player1, Player *player2);
int isCampsite(Server *server, int pos_x, int pos_y);
void leaveTreasure(Player *player, int pos_x, int pos_y);
void printPlayers(Server *server);
void create_box(Server *server, bool flag, int move, Player *player);
void *serverInputThread(Server *server, int ch);
void generateLegend(Server *server);
void generateLegendPlayer();
void updateRounds(Server *server);
void erasePlayer(Player *player);
int init_err_panel(Server *server);
void printPlayerTreasures(Server *server);
void print_err(Server *server,char *msg);
int makeMoveBeast(UserPacket packet);
void printPlayerStats(UserPacket packet);
void printMap(UserPacket packet, char *map_snippet);
int random_move();
int plot_line(int x0, int y0, int x1, int y1);
int distance(int x0, int y0, int x1, int y1);
int makeMovePlayer(UserPacket packet);

#endif //SECOND_TRY_SERVER_UTILS_H
