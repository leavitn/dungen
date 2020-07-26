#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <ncurses.h>

#define HEIGHT_MAX      20	// map height
#define WIDTH_MAX       80	// map width
#define AREA      	  	HEIGHT_MAX * WIDTH_MAX // map area
#define INVALID     	-1
#define SUCCESS			true
#define FAILURE			false
#define MAX_STEPS		999

struct node {
    int key;
    int priority;
    struct node *next;
};

struct room {
	int width, height, coords;
	struct room *next;
};

// Coordinate functions
int hash(int y, int x);   // create hash from x and y coords
int gety(int key);      // derive y coordinate from key
int getx(int key);      // derive x coordinate from key
int offsetkey(int key, int y, int x); // old key + (x + y offsets) = new key if valid
int isvalid_key(int key, int y, int x); // returns whether a key is invalid
int howfar(int from, int to); // measures the manhattan distance between two keys
// misc utility functions
int roll(int ndice, int faces); // roll(2, 4) = roll 2d4
int randint(int min, int max); // rolls a result between min and max number
bool isodd(int x); // returns whether an integer is odd
float probfail(int a, int d); // probability of failing a roll 1da - 1db
float probsucc(int a, int d); // probability of succeeding in a roll 1da - 1db
void arrcpy(int from[], int to[]); // copy contents of an int map array to another
// linked list functions for rooms
void roomlist_append(struct room **list, struct room *r); // add room to room list
void roomlist_purge(struct room **list); // frees all rooms in the room list
void copyRoom(struct room *from, struct room *to); // copies the contents of one room to another
int room_listlen(struct room *list);
// linked list functions for nodes
void nodelist_append(struct node **list, int key); // add node to node list
void nodelist_purge(struct node **list); // frees all rooms in the node list
int nodelistlen(struct node *list); // counts all the members in a linked list
// pathfinding
struct node *astar(int moveCost[], int start, int stop); // a* pathfinding algorithm
