// simple dungeon gen
// attempt to randomly place a bunch of rooms on the screen
// then connect them via carving tunnels

#include "rl.h"

#define MAX_ROOMS		10
#define MAX_ATTEMPTS 	30
#define SPREAD 			1 	// min. # of tiles between rooms. Increasing requires more attempts

enum { STONE, GRANITE, ROOM, BORDER, CORNER, CORRIDOR, O_DOOR, C_DOOR, 
		IRONBARS, WATER, LAVA, LINK, SPACER, UPSTAIRS, DOWNSTAIRS};

//bool printRect(int key, int width, int height); // prints a rectangle 
// randomly place rooms, determine if they fit
void selRoomSize(struct room *r); // select a random rectangle's size
int selRoomPlacement(int height, int width); // select the placement for the room
bool attemptRoom(int draft[], struct room *r); // attempts to place a room
bool attemptBorders(int draft[], struct room *r); // attempts placement of borders
bool attemptSpacers(int draft[], struct room *r); // does room violate min # of tiles between rooms?
// linking rooms together
void connect_rooms(int map[], struct room *roomlist); // connect the rooms on the map with tunnels
void picklinks(int links[], struct room *roomlist); // popular array with room connects
int chooselink(struct room *r); // choose link for room connection
void sortlinks(int links[], int n); // sort the links by distance from the first link
void populate_cost_map(int moveCost[], int map[]); // populate the movecost map for pathfinding
int get_move_cost(int val); // given a mapval, returns a move cost
void connect_links(int map[], int start, int stop); // connect the provided start and stop links on the map
// utility functions for dungeon generation 
void printMap(int map[]); // prints symbol on screen if coords on the map are true
void tunnel(int map[], struct node *head_ref); // carve keys from a list
void carve(int map[], int key); // carves a room out at key
char getsymbol(int val); // returns a symbol based on a given value
int getArea(int map[]); // returns the sum of the map space
bool isborder(int oy, int ox, struct room *r); // returns if border
bool iscorner(int oy, int ox, struct room *r); // returns if corner

int main(void)
{
	struct room r; // room prototype, if it places on the map, a copy is added to the room list
	struct room *roomlist = NULL; // keeps copies of successful room placements
	int draft[AREA]; // working draft of the map, the "what if?"
	int final[AREA]; // where changes are saved to map
	int i, j;
	
	memset(draft, 0, sizeof(draft));
	arrcpy(draft, final);
	initscr(); // initalize ncurses window
	srand(time(0)); // seed random table
	r.next = NULL; // not used for the prototype

	// attempt to place MAX_ROOMS in MAX_ATTEMPTS per room 
	for (i = 0; i < MAX_ROOMS; i++)	 
		for (j = 0; j < MAX_ATTEMPTS; j++)
		{
			selRoomSize(&r); // randomly determine room size
			r.coords = selRoomPlacement(r.height, r.width); // randomly determined valid coordinates
			if (	attemptRoom(draft, &r) == SUCCESS    && 
					attemptBorders(draft, &r) == SUCCESS &&
					attemptSpacers(draft, &r) == SUCCESS 
			   )
			{ // if placement on draft is successful for both rooms and borders
				arrcpy(draft, final); // copy draft to final
				roomlist_append(&roomlist, &r);
				break; // move on to placement of next room up to MAX_ROOMS
			}
			else
				arrcpy(final, draft); // reset draft to last final, attempt again til MAX
		}
	connect_rooms(final, roomlist);
	printMap(final);
	printw("%d", getArea(final));	
	roomlist_purge(&roomlist);
	refresh();
	getch();
	endwin();
	
	return 0;
}

// selects the size of a rectangle
void selRoomSize(struct room *r)
{
	const int MIN_RECT = 3; // offset by minimum allowed height and width
	const int MAX_TYPES = 4; // max types of rectangle dimensions 0 - 4
	const int ODDS_ONLY = 2; // multiplier to choose odds only

	// valid dimensions can be { 3, 5, 7, 9, or 11 }

	r->height = (rand() % (MAX_TYPES - 1) ) * ODDS_ONLY + MIN_RECT;
	r->width = (rand() % MAX_TYPES) * ODDS_ONLY + MIN_RECT;
	return;
} 

// select placement of rectangle
int selRoomPlacement(int height, int width)
{
	int y, x;
	const int MIN_PLACEMENT = 1 + SPREAD;
	const int MAX_OFFSET = 2 + SPREAD; // -1 for start from zero, -1 borders, -SPREAD

	y = rand() % (HEIGHT_MAX - MAX_OFFSET - height) + MIN_PLACEMENT;
	x = rand() % (WIDTH_MAX - MAX_OFFSET - width) + MIN_PLACEMENT; 
	return hash(y, x);
}

// attempts placement of a room to draft array starting at coords "key"
bool attemptRoom(int draft[], struct room *r)
{
	int i, j;
	int dest; // destination

	for (i = 0; i < r->height; i++)
		for (j = 0; j < r->width; j++)
		{
			dest = offsetkey(r->coords, i, j);
			if (dest == INVALID)
				return FAILURE; // something went wrong
			else if(draft[dest] == ROOM)
				return FAILURE; // overlap
			else
				draft[dest] = ROOM;
		}
	return SUCCESS;
}

// attempts placement of a room borders to draft array given coords "key"
bool attemptBorders(int draft[], struct room *r)
{
	int i, j;
	int key, dest; // destination

	key = offsetkey(r->coords, -1, -1); 
	for (i = 0; i < r->height + 2; i++) // y
		for (j = 0; j < r->width + 2; j++) // x
		{
			if (isborder(i, j, r))
			{ // if border
				dest = offsetkey(key, i, j);
				if (dest == INVALID)
					return FAILURE; // something went wrong
				else if(draft[dest])
					return FAILURE; // overlap
				else if (iscorner(i, j, r))
						draft[dest] = CORNER; // if corner
				else
					draft[dest] = BORDER;
			}
		}
	return SUCCESS;
}

// makes sure rooms aren't placed too close together
// determined by SPREAD, i.e. minimum number of tiles between rooms
bool attemptSpacers(int draft[], struct room *r)
{
	int i, j;
	int key, dest; // destination

	if ((key = offsetkey(r->coords, -1 - SPREAD, -1 - SPREAD)) != INVALID)
	{
		for (i = 0; i < r->height + ((1 + SPREAD) * 2); i++) // y
			for (j = 0; j < r->width + ((1 + SPREAD) * 2); j++) // x
			{
				if (	i < SPREAD ||  i > r->height + (1 + SPREAD * 2) - SPREAD || 
						j < SPREAD || j > r->width + (1 + SPREAD * 2) - SPREAD
				   )
				{ // if spacer
					if ( (dest = offsetkey(key, i, j)) != INVALID)
					{ 
						if (draft[dest])
							return FAILURE; // overlap detected
						else
							draft[dest] = SPACER;
					}
				}
			}
		return SUCCESS;
	}
	else
		return FAILURE; // key is invalid, cannot hash
}

// connect the rooms on the map with tunnels
void connect_rooms(int map[], struct room *roomlist)
{
	int n = room_listlen(roomlist);
	int links[n];
	int i, start, stop;
	struct node *path;
	picklinks(links, roomlist);
	sortlinks(links, n); // sorts nodes by distance from the first node
	for (i = 0; i < n; i++)
		map[links[i]] = LINK;

	for (i = 0; i < n; i++)
	{ // for each pair of links, connect them
		start = links[i];
		if (i == n - 1)
			stop = links[0];
		else
			stop = links[i + 1];
		connect_links(map, start, stop);
	}

	return;
}

// populate array with room connection keys
void picklinks(int links[], struct room *roomlist)
{
	struct room *curr;
	int i;
	for (curr = roomlist, i = 0; curr; curr = curr->next, i++)
		links[i] = chooselink(curr);
	return;
}

// given a room, returns a key of a border tile that will be connected to another room
int chooselink(struct room *r)
{
	int n = (r->width * 2 + r->height * 2) - 4; // border tiles less 4 corners
	int choice = rand() % (n - 1); // start count from zero
	int cnt = 0;
	int key = offsetkey(r->coords, -1, -1);
	int i, j = 0;

	for (i = 0; i < r->height + 2; i++)
		for (j = 0; j < r->width + 2; j++)
			if (isborder(i, j, r) && !iscorner(i, j, r)) // if border
			{ // if is border but not a corner
				if (cnt == choice || cnt == n - 1)
					return offsetkey(key, i, j);
				else
					cnt++;
			}
	return offsetkey(key, --i, --j);	// default case if error
}

// sort connection nodes by minimum distance from the first node in the list
void sortlinks(int sorted[], int n)
{
	int min, dist, i, j, pos; // dist = distance, pos = position
	int tmp; // temp container

	for (i = 0; i < n - 1; i++) // for each member in the list
	{
		min = MAX_STEPS;
		for (j = i + 1; j < n; j++) // compare distance between the remaining nodes in the list
		{
			if ((dist = howfar(sorted[i], sorted[j])) < min)
			{ // if distance measured between 2 nodes is shorter than current known min.
				min = dist; // update minimum distance
				pos = j;    // note position of the closest node in list
			}
		}
		tmp = sorted[i + 1]; // swap positions between next node and mate
		sorted[i + 1] = sorted[pos]; // mate is now next in line
		sorted[pos] = tmp; // former position now occupied by former next in line
	}
	return;
}

// connect the provided start and stop links on the map
void connect_links(int map[], int start, int stop)
{
	int costMap[AREA];
	int i;
	struct node *path = NULL;

	memset(costMap, 0, sizeof(costMap));
	for (i = 0; i < AREA; i++)
		costMap[i] = get_move_cost(map[i]);
	costMap[start] = 0;
	costMap[stop] = 0;
	path = astar(costMap, start, stop);
	tunnel(map, path);
	nodelist_purge(&path);

	return;
}

void tunnel(int map[], struct node *head_ref)
{
	struct node *curr;
	for (curr = head_ref; curr; curr = curr->next)
		carve(map, curr->key);
	return;
}

// carves a room at coordinates
void carve(int map[], int key)
{
	int i, j;
	int offset;

	if (map[key] != ROOM) // add room to room map
	{
		map[key] = ROOM;
		// then add borders around the room
		for (i = -1; i < 2; i++)  // y coordinate offset
			for (j = -1; j < 2; j++) // x coordinate offset
				if ( (offset = offsetkey(key, i, j)) != INVALID )
					if (map[offset] != ROOM) // if not a room
						map[offset] = BORDER ; // then add a border
	}
	return;
}


// prints symbol on screen if coords on the map are true
void printMap(int map[])
{
	int i, j, val;

	for (i = 0; i < HEIGHT_MAX; i++)
		for (j = 0; j < WIDTH_MAX; j++)
		{
			val = map[hash(i, j)];
			if (val)
				mvaddch(i, j, getsymbol(val));
		}
	return;
}

int getArea(int map[]) // sums up the map space
{
	int i;
	int sum = 0;
	for (i = 0; i < AREA; i++)
		if (map[i] == ROOM)
			sum++;
	return sum;
}


// returns a symbol based on a given value
char getsymbol(int val)
{
	switch(val)
	{
		case ROOM: case CORRIDOR:	return '.';
		case BORDER: case CORNER: 	return '#';
		case O_DOOR: 				return '\'';
		case C_DOOR: 				return '+';
		case IRONBARS: 				return '=';
		case WATER: case LAVA: 		return '~';
		case UPSTAIRS:				return '>';
		case DOWNSTAIRS:			return '<';
		// debugging
		case LINK: 					return '}';
		case SPACER:				return ' '; 
		default: 					return ' ';
	}
}


// returns if the given coords in the context of the room are a border
bool isborder(int oy, int ox, struct room *r)
{
	// oy = y offset
	// ox = x offset
	return oy == 0 || ox == 0 || oy == r->height + 1 || ox == r->width + 1; 
}

// returns if the given coords in the context of the room are a corner
bool iscorner(int oy, int ox, struct room *r)
{
	// oy = y offset
	// ox = x offset
	return	(oy == 0 && ox == 0) 						|| // nw corner
			(oy == 0 && ox == r->width + 1) 			|| // ne corner
			(oy == r->height + 1 && ox == 0) 			|| // sw corner
			(oy == r->height + 1 && ox == r->width + 1);   // se corner
}

// given a tile type, returns the cost to move to that tile
int get_move_cost(int val)
{
	switch(val)
	{
		case STONE: case SPACER: return 1;
		default: return 4;   
	}
}

// populate the moveCost map to be fed into the pathfinding algorithm
void populate_cost_map(int moveCost[], int map[])
{
	int i;

	for (i = 0; i < AREA; i++)
		moveCost[i] = get_move_cost(map[i]);
	return;
}


/*
// prints a rectangle at origin key, for width and height of rectangle
bool printRect(int key, int width, int height)
{
	int i, j;
	int oy, ox; // origin y and x

	if (key < 0)
		return INVALID;

    oy = gety(key); // origin y
    ox = getx(key); // origin x

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
		    if (oy + i > HEIGHT_MAX || ox + j > WIDTH_MAX)
		        return FAILURE; // safety: make sure key is within dimensions
		    else
				mvprintw(oy + i, ox + j,".");
		}
	return SUCCESS;
}
*/
