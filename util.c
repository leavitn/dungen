// Utility functions
#include "rl.h"

// returns a key from y and x
int hash(int y, int x)
{
    return x * HEIGHT_MAX + y;
}

// given a key, derive the y coordinate
int gety(int key)
{
    return key - HEIGHT_MAX * getx(key);
}

// given a key, derive the x coordinate
int getx(int key)
{
    return key / HEIGHT_MAX; // truncates decimal because it returns an int
}

// returns a key transformed from the given key and the x and y offset values
int offsetkey(int key, int y, int x)
{
    int oy = gety(key); // origin y
    int ox = getx(key); // origin x
 
    if (oy + y >= HEIGHT_MAX || ox + x >= WIDTH_MAX || oy + y < 0 || ox + x < 0)
        return INVALID; // if key is invalid, hash won't work
    else
    	return hash(oy + y, ox + x); 
}

// returns whether a key is invalid or not - not used currently
int isvalid_key(int key, int y, int x)
{
    int oy = gety(key); // origin y
    int ox = getx(key); // origin x
 
    if (oy + y > HEIGHT_MAX || ox + x > WIDTH_MAX || oy + y < 0 || ox + x < 0)
        return false; // key is not within dimensions
    else
    	return true; // key is valid
}

// measures the manhattan distance between two keys
int howfar(int from, int to)
{
	return abs(getx(from) - getx(to)) + abs(gety(from) - gety(to));
}


// rolls ndx dice
int roll(int ndice, int faces)
{
	return rand() % faces + ndice;
}

// rolls a result between min and max number
int randint(int min, int max)
{
	return rand() % max + min;
}

// returns whether an integer is even or odd
bool isodd(int x)
{
	return x & 1; // mask returns if last digit of binary is 1 = odd
}

// triangle number = x! but with summation instead of products
// used for dice probability
int trinum(int x)
{
	return (x * (x + 1)) / 2; // (x - 0 ) + (x - 1) + ... (x - n)
}

// probability of failing a roll, %chance 1dx - 1dy > 0
// a = attacker's roll, d = defenders roll
float probfail(int a, int d)
{ 
	int n = trinum(d); // n roll combos that result in failure

	if (d > a)
		n = n - trinum(d - a); // not all combinations are exhausted in this case

	return (float) n / (a * d); // same as n * (1.0 / a) * (1.0 / d)
	// probability of rolling any one dice face * n roll combos that result in failure
}

// probability of success
// %chance 1dx - 1dy > 0
// a = attacker's roll, d = defenders roll
float probsucc(int a, int d)
{
	return 1 - probfail(a, d);
}

// copy contents of an int array from one to another
void arrcpy(int from[], int to[])
{
	int i;

	for (i = 0; i < AREA; i++)
		to[i] = from[i];

	return;
}


// add room to room list
void roomlist_append(struct room **list, struct room *r)
{
	struct room *curr;
	struct room *new = malloc(sizeof(struct room));
	copyRoom(r, new);

	if (*list) // if list has members
	{
		for (curr = *list; curr->next != NULL; curr = curr->next)
			;
		curr->next = new;	
	}
	else
		*list = new; // if list is empty, start a new list
	return;
}

// frees all rooms in the room list
void roomlist_purge(struct room **list)
{
    struct room *curr;

    if ( (*list) == NULL) // if queue is empty, return
        return;
    else if ( (*list)->next == NULL) // if only 1 member in queue, free ref and point to NULL
    {
        free(*list);
        *list = NULL;
        return;
    }
    else // if more than 1 member in the queue
    {
        for (curr = *list; curr->next->next != NULL; curr = curr->next)
            ;
        free(curr->next);
        curr->next = NULL;
    }
    roomlist_purge(list); // recur until list == NULL
    return;
}

// copies the contents of one room to another
void copyRoom(struct room *from, struct room *to)
{
	to->width = from->width;
	to->height = from->height;
	to->coords = from->coords;
	to->next = from->next;
	return;
}

// counts all the members in a linked list
int room_listlen(struct room *list)
{
	struct room *curr;
	int cnt = 0;

	for (curr = list; curr; curr = curr->next)
		cnt++;
	return cnt;
}

// add room to room list
void nodelist_append(struct node **list, int key)
{
	struct node *curr;
	struct node *new = malloc(sizeof(struct node));
	new->key = key;
	new->next = NULL;
	new->priority = 0;

	if (*list) // if list has members
	{
		for (curr = *list; curr->next != NULL; curr = curr->next)
			;
		curr->next = new; // append list
	}
	else
	{
		*list = new; // if list is empty, start a new list
	}
	return;
}

// frees all rooms in the room list
void nodelist_purge(struct node **list)
{
    struct node *curr;

    if ( (*list) == NULL) // if queue is empty, return
        return;
    else if ( (*list)->next == NULL) // if only 1 member in queue, free ref and point to NULL
    {
        free(*list);
        *list = NULL;
        return;
    }
    else // if more than 1 member in the queue
    {
        for (curr = *list; curr->next->next != NULL; curr = curr->next)
            ;
        free(curr->next);
        curr->next = NULL;
    }
    nodelist_purge(list); // recur until list == NULL
    return;
}

// counts all the members in a linked list
int node_listlen(struct node *list)
{
	struct node *curr;
	int cnt = 0;

	for (curr = list; curr; curr = curr->next)
		cnt++;
	return cnt;
}