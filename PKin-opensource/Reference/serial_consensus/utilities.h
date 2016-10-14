#ifndef UTILITIES_H
#define UTILITIES_H

extern int EMPTY_POSSIBILITIES[25];

/**************************
deletes an item from a given linked list and uses the freer to release memory
 **************************/
void deleteItem(list * ll, void * item, void * freer(void *));


void readSelectedData(const char * filename, int droplocus);

void datacleaner(void * p) ;

void initMother();

void init();

int * lookupPossibilities(Pair * p);

void copyList(list * target, const list * source);

static unsigned int calculateHash(sibgroup * group);

int getPairInt(Pair * pair);

int pairEquals(Pair * p1, Pair * p2);

int sibgroupEquals(sibgroup * group1, sibgroup * group2);

 unsigned int sibgroupHash(void * group);
/******************************
assigns an individual to a group, if successful without any modification to acceptability it returns 1
if it can be accepted but w/o acceptability it returns 2
if no way to accept then returns 0
 ****************************/
int assignIndividualToGroup(sibgroup * group, individual * indi);

/******************************************************
   Writes results to file in a tab separated format
 ****************************************************/
void writeResultsFile(const char * filename);


/**********************************
cleans up everything from memory
***********************************/
void cleanUp();

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
int equals(int* x, int* y);

/*********************
returns the value of the int as hash for hash table

params: x the value which is simply returned
**********************/
unsigned
 int  getIntHash(int* x);

/****************************************************************
Initializes a sibgroup

params:
group: pointer to the sibgroup which will be initialized
***************************************************************/
void initializeSibgroup(sibgroup * group);



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
sibgroup* copySibgroup(sibgroup * source);


/*************************************************************
 Frees memory used by a sibgroup
************************************************************/
  void deleteSibgroup(sibgroup * group);

/**********************
 deletes a given list with and using the given 'cleaner' function to free up data

*******************************************/
void deleteList(list * target, void * cleaner() );


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
int assignIndividualToGroup(sibgroup * group, individual * indi);

#endif
