# Labyrinth Treasure Hunt - multiplayer terminal game
Multiplayer game based on TCP sockets communication with multithreading.

# Introduction
"Labyrinth Treasure Hunt" is an exciting board game designed for four independent players who play simultaneously. The game revolves around players navigating through a labyrinth to collect treasures in the form of coins. The objective is to gather a sufficient number of treasures and deliver them to the campsite, while facing challenges and strategic decision-making.

# Gameplay:
* Each player, represented by a unique character, starts at their designated starting point within the labyrinth.
* The players must navigate through the maze, using different strategies and tactics to collect treasures.
* Treasures appear randomly throughout the maze as coins (c), small treasures (t), and valuable treasures (T).
* The players can carry any number of coins at a time, but they risk losing them in encounters with wild beasts or collisions with other players.
* Wild beasts (*) roam the maze and can pose a threat to players. If a player is attacked, they die, and the loot they carried is left at the location of their demise (D). The player then respawns at their starting point.
* Colliding with another player results in both players' loot being left at the collision location (D), and both players respawning at their respective starting points.
* The loot left at a location (D) holds its value, which is the sum of the coins carried by both players involved in a collision.

# Game Characters:
* The game features two types of players: human players (HUMAN) and computer-controlled players (bot/CPU).
* Human players control their character's movement using arrow keys (up, down, left, right).
* Bot players navigate autonomously using random movement.

# Wild Beast Behavior:
* The game includes one type of opponent, the "wild beast" (*), which behaves as follows:
  * If the beast doesn't have a player within its line of sight, it can move aimlessly in any direction or stay in place.
  * The beast moves at the same speed as the players, allowing for one move per turn.
  * If a player enters the beast's line of sight, it pursues the player until the player escapes its field of view. After losing sight of the player, the beast may resume aimless movement or return to the last seen location of the player.

# Field of View:
* The server doesn't provide the players with a complete map of the maze. Instead, it only shares the data within the player's field of view.
* Each player's field of view is a circular area with a radius of two squares around their current position.
* Along with the field of view, the server provides the player's coordinates in the game world. Players must build their own map based on this information.

# Joining and Leaving the Game:
* The server can accommodate a maximum of four players. If a user attempts to start an additional player (client process), they will receive a message indicating that the server is full, and their process will terminate.
* For each newly arrived player, a random spawn location (XY coordinates) is assigned. The player appears at this location at the start of the game and after each death.
* Human players and bot players are treated equally from the server's perspective, ensuring a unified API interface between the server and its clients.
* The server handles situations where a player naturally leaves the game (e.g., pressing Q on the client side) or abruptly disconnects (process termination). During gameplay, players are able to disconnect and later reconnect to a vacant spot if available.

# Additional specification
* Server, beast client, player clients and bot clients are separate processes:
  * Each player and bot are run as an individual process, allowing for concurrent execution and independent decision-making.
  * Beasts are implemented as a single process with multiple threads, where each beast corresponds to one thread.

# Screenshots

## Server view
![](https://github.com/dabkowski/Multiplayer-terminal-game/blob/main/assets/server_view.gif)

## Client view
![](https://github.com/dabkowski/Multiplayer-terminal-game/blob/main/assets/client_view.gif)

## Beast attack
![](https://github.com/dabkowski/Multiplayer-terminal-game/blob/main/assets/beast_attack.gif)
