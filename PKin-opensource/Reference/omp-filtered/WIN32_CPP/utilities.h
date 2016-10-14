#ifndef UTILITIES_H
#define UTILITIES_H

extern int EMPTY_POSSIBILITIES[25];
extern int blocks;
void cleanUpOverlaps(list * source,int x);
list * newList();
void addToSortedList(list * x, int);
void writeGamsFileFromSetsList(const char * filename, list * setslist);
unsigned int * encodeList(list * source);
struct hashtable * reconstruct(int *lociorder, int nloci );
static unsigned int calculateHash(sibgroup * group, int nloci);
void addToVector(vector * vec, unsigned int ** intlist);
struct hashtable* itersectsibgroups(list * currentList, list * locusitemsets);
list * lociConstruct(const char * filename, int loci);
int equalsIntList(void * intlist1, void * intlist2, int dummy_unused);
unsigned int getIntListHash(void * intlist_param);
/**************************
deletes an item from a given linked list and uses the freer to release memory
 **************************/
void deleteItem(list * ll, void * item, void * freer(void *));

void blank(void *k);

void readSelectedData(const char * filename, int droplocus);

void datacleaner(void * p) ;

void initMother();

int * lookupPossibilities(Pair * p);

void copyList(list * target, const list * source);

//static unsigned int calculateHash(sibgroup * group);

unsigned int getPairInt(void * pair);

int pairEquals(void * p1_param, void * p2_param, int dummy_unused);

int sibgroupEquals(void * group1, void * group2, int nloci);

 unsigned int sibgroupHash(void * group);
/******************************
assigns an individual to a group, if successful without any modification to acceptability it returns 1
if it can be accepted but w/o acceptability it returns 2
if no way to accept then returns 0
 ****************************/
int assignIndividualToGroup(sibgroup * group, individual * indi, int * lociorder, int nloci);

/******************************************************
   Writes results to file in a tab separated format
 ****************************************************/
void writeResultsFile(const char * filename, struct hashtable * x);


/**********************************
cleans up everything from memory
***********************************/
void cleanUp(struct hashtable *);

/************************************************************************************
 adds a new item to a given doubly linked list using the 'node' data structure
 which must be deallocated later

 params:
 addTo: pointer to the list to which the item will be added
 item: pointer to the item that is to be added to the list
********************************************************************************/
void addToList(list * addTo, void * item);

/****************************************************************
 reads data into the linked list 'data'

 params:
 filename: name of the file to read
****************************************************************/
void readData(const char * filename);

// equality operator
int equals(void* x, void* y, int dummy_unused);

/*********************
returns the value of the int as hash for hash table

params: x the value which is simply returned
**********************/
unsigned
 int  getIntHash(void* x);

/****************************************************************
Initializes a sibgroup

params:
group: pointer to the sibgroup which will be initialized
***************************************************************/
void initializeSibgroup(sibgroup * group, int nloci);



/****************************************************************************
* computes the intersection of two sets of possibilities in form of pointers
* to integers arrays of size MAX_POSSIBILITIES
*
* params:
* set1: first possibilities set
* set2: second possibilities set
**************************************************************************/
int * intersectPossibilities(int* set1, int * set2);

/**************************************************************
 copies integer values from one integer array to another, 
	both size MAX_POSSIBILITIES
***************************************************************/
void copyPossibilities(int * target, const int * source);

/**************************************************************
 creates a deep copy of a given sibgroup and returns it
***************************************************************/
sibgroup* copySibgroup(sibgroup * source, int nloci);


/*************************************************************
 Frees memory used by a sibgroup
************************************************************/
void deleteSibgroup(sibgroup * group, int nloci);

/**********************
 deletes a given list with and using the given 'cleaner' function to free up data

*******************************************/
void deleteList(list * target, void cleaner( void* ) );


/*******************************************
 newAlleleMap
**************************************************/
struct hashtable * newAlleleMap( int x );


/****************************************************
 Assigns an Individual to Sibgroup

returns 0 if assignment was not possible, 1 if assignment is
possible without any change to the existing alleles of group
2 if it requires a change

the given group IS modified, therefore a copy may be made 
prior to calling this function
******************************************************/
//int assignIndividualToGroup(sibgroup * group, individual * indi);

#endif
