# Nuggets Game
## CS50 Software Design and Implementation at Dartmouth College

### Overview
This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.
Our map file is located in the `maps` folder, named `add-drop.txt`.

### Run Instructions
In order to make the client and server, simply type in `make all` into the command line, 
as instructed in the Makefile. To test, simply type `./server 2>server.log maps/mapfile.txt`
with the name of some map as the mapfile. Once the server announces the port number, in another
window run `./client 2>player.log hostname port playerName` or just `./client 2>spectator.log hostname port`
depending on if you want to be a player or spectator. To run valgrind, type `valgrind` or `myvalgrind`
followed by the appropriate test arguments.

