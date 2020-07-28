/******************************************************************************
 
Pathfinding algorithms

*******************************************************************************/
 
#include "rl.h"

#define CARDINALS 	5	// n, s, e, w
#define ALLDIRS		9 	// n, s, e, w, ne, nw, se, sw

/* #################### FUNCTIONS ############################### */
int *create_Djikstra_Map(int moveCost[], int start); // uses djikstra pathfinding to return a path map
// utility functions
int x(int i); // given an iteration, return an x coord
int y(int i); // given a key, return a y coord
void init(int *map, int val); // initialize map
void fprintArray(int array[], int step);
bool isValid(int key); // returns whether a key is valid
void report(int cameFrom[], int stop); // record output (path)
struct node *pathtolist(int cameFrom[], int stop); // writes path from array data into a linked list
// priority queue functions
struct node *newNode(int key, int priority); // creates a node with value key and priority
void pqueue_push(struct node **queue, int key, int priority); // push a new key to the end of the queue
int pqueue_pop(struct node **queue); // pop the priority queue
void pqueue_purge(struct node **queue); // free()s all remaining nodes in the queue
/* ############################################################## */

// a* pathfinding algorithm
// one to one
struct node *astar(int moveCost[], int start, int stop)
{ 
    struct node *frontier;  // priority queue of cells to visit
    int costTo[AREA];       // map of cumulative cost from start (origin) to key (hash of coords)
    int cameFrom[AREA];     // the cell this cell was visited from originally
    int parent, child;      // stores keys, parent = visited key, child = key visitable from parent key (adjacent)
    int i;                  // iterator

    // Initialization
    pqueue_push(&frontier, start, 0); // priority queue starts with start
    init(costTo, MAX_STEPS);             // initialize costTo map
    costTo[start] = 0; // current tile (start) is 0 steps away
    cameFrom[start] = INVALID; // so you know it's the start

    while(frontier != NULL)
    {
        parent = pqueue_pop(&frontier); // pop top of the queue
        // visit parent. For each adjacent cell (child), update costTo[child] if it can be lowered
        //      and if so, add to piority queue so key can be visited later
        for (i = 1; i < CARDINALS; ++i)
        { // ALLDIRS for all 8 dirs, CARDINALS for 4 cardinal directions only
            child = offsetkey(parent, y(i), x(i));
            // if not out of bounds and cost from start to curr to tmp < recorded costTo[tmp]
            // updated costTo[tmp] to lower value and add to priority queue with priority = costTo[tmp]
            if (child == stop) // found goal?
            {
            	costTo[child] = costTo[parent] + moveCost[child]; // update costTo map
            	cameFrom[child] = parent;
            	// report(cameFrom, stop); // for debugging
                pqueue_purge(&frontier);   // purge the rest of the queue
            	return pathtolist(cameFrom, stop);
            }
            else if (isValid(child) && costTo[parent] + moveCost[child] < costTo[child])
            { 
                costTo[child] = costTo[parent] + moveCost[child]; // update costTo map
                cameFrom[child] = parent; // update cameFrom
                pqueue_push(&frontier, child, costTo[child] + howfar(parent, child)); 
                // push key to the queue, priority = costTo
                // this means that a key could exist in the queue multiple times with different priorities
                // by the time it visits the key for the last time, there will be no cells to visit.
                // Can add a function that would seek & destroy prexisting keys in the queue, but would that
                // really speed this up?
            }
        }
    }
	return NULL; // failure to path find, should log this
}

// like a*, except flood fills to every legal tile in them map
// one to many
// needs to be modified to return the path map
int *create_Djikstra_Map(int moveCost[], int start)
{ 
    struct node *frontier;  // priority queue of cells to visit
    int costTo[AREA];       // map of cumulative cost from start (origin) to key (hash of coords)
    int cameFrom[AREA];     // the cell this cell was visited from originally
    int parent, child;      // stores keys, parent = visited key, child = key visitable from parent key (adjacent)
    int i;                  // iterators
    int stop;

    // Initialization
    pqueue_push(&frontier, start, 0); // priority queue starts with start
    init(costTo, MAX_STEPS);             // initialize costTo map
    costTo[start] = 0; // current tile (start) is 0 steps away
    cameFrom[start] = INVALID; // so you know it's the start
    // make djikstra steps map
    while(frontier != NULL)
    {
        parent = pqueue_pop(&frontier); // pop top of the queue
        // visit parent. For each adjacent cell (child), update costTo[child] if it can be lowered
        //      and if so, add to piority queue so key can be visited later
        for (i = 1; i < ALLDIRS; ++i)
        { // for each of the 8 directions - change i < 5 for cardinal only
            child = offsetkey(parent, y(i), x(i));

            // if not out of bounds and cost from start to curr to tmp < recorded costTo[tmp]
            // updated costTo[tmp] to lower value and add to priority queue with priority = costTo[tmp]
            if (isValid(child) && costTo[parent] + moveCost[child] < costTo[child])
            { 
                costTo[child] = costTo[parent] + moveCost[child]; // update costTo map
                cameFrom[child] = parent; // update cameFrom
                pqueue_push(&frontier, child, costTo[child]); // push key to the queue, priority = costTo
                // this means that a key could exist in the queue multiple times with different priorities
                // by the time it visits the key for the last time, there will be no cells to visit.
                // Can add a function that would seek & destroy prexisting keys in the queue, but would that
                // really speed this up?
            }
        }
    }
    fprintArray(costTo, 1);

} 

// writes path from array data into a linked list
struct node *pathtolist(int cameFrom[], int stop)
{
    int curr, i;
    int tmp[AREA];
    struct node *path = NULL;

    memset(tmp, 0, sizeof(tmp));

    // record path from stop to start into a temp array
    for (curr = stop, i = 0; curr != INVALID; curr = cameFrom[curr], i++)
        tmp[i] = curr;
    
    i--;
    while (i >= 0)
    { // write temp array values to list in reverse order (start to stop)
        nodelist_append(&path, tmp[i]);
        i--;
    }
    
    return path;
}


// record output (path) - for debugging
void report(int cameFrom[], int stop)
{
	int curr = stop, count = AREA;
	int path[AREA];
	memset(path, 0, sizeof(path));
	path[stop] = count;
	while (cameFrom[curr] != INVALID)
	{
		path[curr] = --count;
		curr = cameFrom[curr];
	}
	fprintArray(path, 1);
    return;
}

// initialize map with a value so all cells are set to value
void init(int *map, int val)
{
    int i;
    for (i = 0; i < AREA; ++i)
        map[i] = val;
    return;
}

// returns whether a key is valid
bool isValid(int key)
{
    return key > INVALID && key < AREA;
}
 
// for a given iteration return the y coord
int y(int i)
{
    switch(i) {
        case 1: return 0;   // e
        case 2: return 0;   // w
        case 3: return 1;   // n
        case 4: return -1;  // s
        case 5: return 1;   // se
        case 6: return -1;  // nw
        case 7: return -1;  // ne
        case 8: return 1;   // sw
    }
}
 
// for a given iteration return the x coord
int x(int i)
{
    switch(i) {
        case 1: return 1;   // e
        case 2: return -1;  // w
        case 3: return 0;   // n
        case 4: return 0;   // s
        case 5: return 1;   // se
        case 6: return -1;  // nw
        case 7: return 1;   // ne
        case 8: return -1;  // sw
    }
}
 
// Priority Queue functions

// create a new linked list for supplied key and priority
struct node *newNode(int key, int priority)
{
    struct node *tmp;
 
    tmp = malloc(sizeof(struct node));
    tmp->key = key;
    tmp->next = NULL;
    tmp->priority = priority;
    return tmp;
}

// push a new key to the priority queue - sorted MIN to MAX
void pqueue_push(struct node **queue, int key, int priority)
{
    struct node *curr; // current member
    struct node *tmp;

    // if queue doesn't exist, start one
    if (*queue == NULL) 
    {
        *queue = newNode(key, priority);
    }
    // else if need to insert at beginning of the queue
    else if ((*queue)->priority > priority)
    {
        tmp = *queue;
        *queue = newNode(key, priority);
        (*queue)->next = tmp;
    }
    // else find where to insert
    else
    {
        curr = *queue;
        while (curr->next != NULL && priority >= (curr->next)->priority)
                curr = curr->next; // find where to insert the node, sorted priority MIN to MAX
        tmp = curr->next;
        curr->next = newNode(key, priority);   // insert into queue
        curr->next->next = tmp;
    }
    return;
}

// pop the priority queue
int pqueue_pop(struct node **queue)
{
    
    struct node *tmp;
    int key;
 
    if (*queue == NULL)
        return 0;
    else
    {
        key = (*queue)->key;     // for the first in line - get their key
        tmp = *queue;            // temporarily hold the ref to first in line
        *queue = (*queue)->next; // 2nd item in list is now the 1st item
        free(tmp);               // free the 1st in line
        return key;              // 1st in line key is returned
    }
} 

// frees remaining nodes in the queue
void pqueue_purge(struct node **queue)
{
    struct node *curr;
    int i;

    if (*queue == NULL) // if queue is empty, return
        return;
    else if ( (*queue)->next == NULL) // if only 1 member in queue, free ref and point to NULL
    {
        free(*queue);
        *queue = NULL;
    }
    else // if more than 1 member in the queue
    {
        for (curr = *queue; curr->next->next != NULL; curr = curr->next)
            ;
        free(curr->next);
        curr->next = NULL;
    }
    pqueue_purge(queue); // recur until *queue == NULL
}

// save a .csv file of the array
void fprintArray(int array[], int step)
{
    FILE *fp;
    char tmp1[20] = "step";
    char tmp2[2];
    int x, y;

    // create file if it doesn't exist with file name
    tmp2[0] = step + 48; // add step # to file name, 48 = code for '0'
    tmp2[1] = '\0'; // terminating string to use w/ strcat
    strcat(tmp1, tmp2); // concatenate "step" + step #
    strcat(tmp1, ".csv"); // add ".cvs" file extension to file name
    fp = fopen(tmp1, "w");    // create file w/ file name
    for (y = 0; y < HEIGHT_MAX; y++)  // write the array values in csv format
    {
        for (x = 0; x < WIDTH_MAX; x++)
        {
            fprintf(fp, "%d", array[hash(y, x)]);
            if (x < WIDTH_MAX - 1)
                fprintf(fp,",");    
        }
        if (y < HEIGHT_MAX - 1) 
            fprintf(fp, "\n"); // write for all but the final line 
    }
    fclose(fp);
}

