/***********************************************************************
 * File: utilities.c
 * Author: saad sheikh
 *
 * This file contains implementation of various utility functions and
 * all the implementation
 **********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#include "hashtable.h"
#include "hashtable_itr.h"
#include "structures.h"
#include "main.h"
#include "utilities.h"

#define errors stderr
#define dumping 0


int dumpgroupcounter = 0;
FILE * dumpfile = NULL;
int ** pmovemap;

int EMPTY_POSSIBILITIES[25]={0};

/**************************************************
 * Reads Move Map from file
 *************************************************/
void populateMoveMap()
{
  pmovemap = calloc(MAX_ALLELES_PER_LOCUS, sizeof(int*));
  int i;
  for(i = 0; i < MAX_ALLELES_PER_LOCUS; i ++)
    {
      pmovemap[i] = calloc(MAX_POSSIBILITIES, sizeof(int));
    }
  FILE * infile = fopen( "movemap", "r");
  char *temp = malloc(sizeof(char*) * 512);
  int read = fgets(temp, 512, infile);
  char * x, *y, *z;
  char * delims = ",;\n ";
  char * progress;
  while(read > 0 && infile)
    {
      x = strtok_r(temp, delims, &progress);
      int i = atoi(x);

      y = strtok_r(NULL, delims, &progress);
      int j = atoi(y);

      z = strtok_r(NULL, delims, &progress);
      int k = atoi(z);

      pmovemap[i][j]=k;
      
      read = fgets(temp, 512, infile);
      
    }
  free(temp);
  fclose(infile);
}

/****************************************************************
 * adjustAlleles
 ***************************************************************/
int adjustAlleles(sibgroup * group, int locus, int allele)
{
  struct hashtable * allelemap = group->intmaps[locus];
  group->allelecounts[locus]++;

  int newindex = 0;
  int count = hashtable_count(allelemap);
  int * key, * value;
  struct hashtable_iterator * itr = hashtable_iterator(allelemap);
  do
    {
      key = hashtable_iterator_key(itr);
      value = hashtable_iterator_value(itr);
      if(*key < allele)
	{
	  newindex ++;
	}
    }while(hashtable_iterator_advance(itr));
  free(itr);


  int * newkey = malloc(sizeof(int));
  int * newvalue = malloc(sizeof(int));

  *newkey = allele;
  *newvalue = newindex;

  int i = 0;
  for( i = count -1 ; i >= newindex ; i --)
  {
    int * possibs = group->possibilitysets[locus];
    itr = hashtable_iterator(allelemap);
    do{
      key = hashtable_iterator_key(itr);
      value = hashtable_iterator_value(itr);

      }while(hashtable_iterator_advance(itr) && *value != i );
    (*value)++;
    free(itr);
    int j=1;
    int * newpossibs = calloc(MAX_POSSIBILITIES, sizeof(int));
    for( ; j < MAX_POSSIBILITIES; j++)
      {
	if(possibs[j])
	  {
	    if(pmovemap[i][j] > 0)
	      newpossibs[pmovemap[i][j]]=1;
	  }
	else
	  {
	    possibs[j] = 0;
	  }
      }    
    free(possibs);
    group->possibilitysets[locus]=newpossibs;
  }

  hashtable_insert(allelemap, newkey, newvalue);
  if(*newvalue >= MAX_ALLELES_PER_LOCUS)
    {
      printf(" ");
    }
  return *newvalue;
}

/*****************************************************************
 * Logger function, prints the contents of a sibgroup
 ****************************************************************/
void dumpGroup(sibgroup * group)
{
  struct hashtable_iterator * itr = hashtable_iterator(sibgroupsmap);
  //  sibgroup * group =hashtable_iterator_key(itr);
  int i = 0;
  //  static int groupcounter;

  fprintf(dumpfile, "\"Set(%d):\"", dumpgroupcounter ++);
  //    node * p = group->assignments->head;
  for(i = 0; i < individuals; i ++)
    {

      //      if(p && ((int) p->data) == i)
      if(searchList(group->assignments, i))
	//        p = p->next;
	{
	  fprintf(dumpfile, "\t%d", 1);

	}
      else
	fprintf(dumpfile, "\t%d", 0);
    }
  fprintf(dumpfile, "\n");


}
/******************************************
 Prints details of a sibgroup's internal structures to the given file
 ******************************************/
void logGroup(FILE * logfile, sibgroup * group)
{
  fprintf(logfile, "Set(%d)\n", group->mark);
  fprintf(logfile, "Founders x:%d, y:%d\n", group->founder.x, group->founder.y);
  int locus = 0;
  struct hashtable_iterator * itr;
  
  for(locus = 0; locus < nloci; locus ++)
    {
      fprintf(logfile, "Locus: %d\nAllele Mappings:\n", locus);
      struct hashtable * allelemap = group->intmaps[locus];
      
      itr = hashtable_iterator(allelemap);
      do{
	int * key = hashtable_iterator_key(itr);
	int * value = hashtable_iterator_value(itr);
	fprintf(logfile, "%d => %d\t", *key, *value);
      }while(hashtable_iterator_advance(itr));
      int * possibs = group->possibilitysets[locus];
      fprintf(logfile, "\nPossibilities:\n");
      int i;
      for(i = 1; i < MAX_POSSIBILITIES; i ++)
	{
	  if(possibs[i])
	    {
	      fprintf(logfile, "%d\t", i);
	    }
	}
      fprintf(logfile, "\n");
    }
  fprintf(logfile, "\n===========================\n");
}


/*****************************************
assumes empty target
 *************************/
void copyList(list * target, const list * source)
{
  target->count = source->count;
  node *p = source->head;
  if(p == NULL)
    return;
  node * newnode = malloc(sizeof(node));
  node * oldnode;
  target->head = newnode;
  target->current = newnode;
  //  newnode = p;
  newnode->previous = NULL;
  newnode->data=p->data;
  oldnode = newnode;
  p = p->next;
  while(p != NULL)
    {
      newnode = malloc(sizeof(node));
      newnode->previous = oldnode;
      newnode->data = p->data;
      oldnode->next = newnode;

      oldnode = newnode;
      p = p->next;
    }
  newnode->next = NULL;
}


/*******************************************************************
 Generates initial pair groups
 *******************************************************************/
void generateInitialGroups()
{
  int count = 0;
  int recurrences =0;
  int i = 0;
  node * current = data.head;
  sibgroupsmap = (struct hashtable *) create_hashtable(individuals * individuals, sibgroupHash, sibgroupEquals); 
  for(i = 0; i < individuals; i ++, current = (*current).next )
    {
      node * current2 = (*current).next;
      int j;
      for(j = i+1; j < individuals; j++, current2 = (*current2).next)
	{
	  if(i == 30 && j == 47)
	    printf(" ");
	  sibgroup * group = malloc(sizeof(sibgroup));
	  initializeSibgroup(group);

	  int assignment = assignIndividualToGroup(group,(individual*) (*current).data);
	 
	  if(assignment == 0)
	    {
		printf("ERROR: UNABLE TO GENERATE INITIAL SINGLETON");
		exit(-1);
	    }
	  int assignment2 = assignIndividualToGroup(group, (individual*) (*current2).data);
	  if(assignment2 == 0)
	    {
		printf("ERROR: UNABLE TO GENERATE INITIAL GROUP");
		exit(-1);
	    }
	  group->founder.x = i;	  
	  group->founder.y = j;
	  addToSortedList(group->assignments, i);
	  addToSortedList(group->assignments, j);
	  calculateHash(group);
	  sibgroup * exist;
	  if(NULL == (exist = hashtable_search(sibgroupsmap, group)))
	    {
	      hashtable_insert(sibgroupsmap, group, group);
	      count ++;
	    }
	  else
	    {
	      recurrences ++;
	      if(!searchList(exist->assignments, i))
		addToSortedList(exist->assignments, i);
	      if(!searchList(exist->assignments, j))
		addToSortedList(exist->assignments, j);
	      //	      exist->assignments[i]=1;
	      //exist->assignments[j]=1;
	      deleteSibgroup(group);
	    }
      }
    }
  if(dolog)
    {
      printf("Done generating initial groups: %d Reccurences avoided: %d\n", count, recurrences);
    }
}

/**************************************************************
 * Sorts by Allele Frequency
 *************************************************************/
void sortLociByAlleleFrequency()
{
  int i = 0;
  int j = 0;
  lociorder = calloc(nloci, sizeof(int));
 for( i = 0; i < nloci; i ++)
   {
     lociorder[i] = i;
   }

 for( i = 0; i < nloci; i ++)
    for(j = i +1; j < nloci; j ++)
      {
	if(hashtable_count(alleles[lociorder[i]]) < hashtable_count(alleles[lociorder[j]]))
	  {
	    int temp = lociorder[i];
	    lociorder[i] = lociorder[j];
	    lociorder[j]= temp;
	  }
      }
}

/***********************************************************
 *  The main 2-allele sibling reconstruction function
 ************************************************************/
void reconstruct( )
{
  int changing = 0;
  int ecount = 0;
  int icount = 0;
  int old =0;
  int new=0;


  generateInitialGroups();

  do
    {
      changing = 0;

      if(dolog)
	printf("External while: %d, no of groups here: %d\n", ecount ++,  hashtable_count(sibgroupsmap) );

      struct hashtable_iterator * itr;
      itr = hashtable_iterator(sibgroupsmap);
      int groupcount = 0;
      
      do{
	   
      sibgroup * group = hashtable_iterator_key(itr); 
     
	if(dolog)
	  printf("Next Group: %d\n", groupcount++);
	if(!(*group).done)
	  {
	    int i = 0;
	    node * current = data.head;
	    if(group->founder.x == 1 && group->founder.y==2)
	      {
		int sdfklsajf= 213;
	      }

	    for(i = 0; i < individuals && current != NULL; i ++, current = (*current).next )
	      {

		if(!(searchList((*group).assignments,i)))
		  {
		    sibgroup * temp = copySibgroup(group);

		    int result = assignIndividualToGroup(temp, (individual *) (*current).data);
		    if(result == 1)
		      {
			assignIndividualToGroup(group,  (individual *) (*current).data);
			addToSortedList((*group).assignments,i);
			deleteSibgroup(temp);
			if(dolog)
			  printf("successful assignment: %d\n", icount ++ );
		      }
		    else if (result == 2)
		      {
			calculateHash(temp);
			sibgroup * search;
			group->mark = 1;
			if( NULL == (search = hashtable_search(sibgroupsmap, temp)))
			    {
			      hashtable_insert(sibgroupsmap, temp, temp);
			      changing = 1;
			      if(dolog)
				printf("New Mutation assignment %d\n", new ++);
			    }
			else{
			  if(!searchList(search->assignments, i))
			    addToSortedList((*search).assignments, i);
			  deleteSibgroup(temp);
			  if(dolog)
			      printf("Old Mutation assignment %d\n", old ++);
			  
			}
		      }
		    else
		      {
			if(dolog)
			  printf("can't assign %d\n", i);
			deleteSibgroup(temp);
		      }
		  
		    if(dolog)
		      {
			printf("that was individual: %d\n", i);
		      }
		  }else
		  {
		    if(dolog)
		      printf("Already assigned %d \n", i);
		  }
	      }
	    (*group).done = 1;
	    if(group->mark )
	      {
		//		dumpGroup(group);

		//	hashtable_remove(sibgroupsmap, group);
		//deleteSibgroup(group);
	      }
	  }
	else{
	  
	  if(dolog)
	    printf("ALREADY DID GROUP\n");
	  if(group->mark && dumping)
	    {
	      dumpGroup(group);
	      hashtable_remove_clear(sibgroupsmap, group, 0);
	      deleteSibgroup(group);
	    }
	}
      }while(hashtable_iterator_advance(itr));
      free(itr);
      printf("Number of groups at loop: %d\n", hashtable_count(sibgroupsmap));
    }while(changing);

  // now add singletons
  

}



/******************************************************
 * determines if the two passed sibgroups are equal
 *****************************************************/

int sibgroupEquals(sibgroup * group1, sibgroup * group2)
{
  
  struct hashtable_iterator * itr;
  int i,j,k;
  int*key, *v;

  if((*group1).hash != (*group2).hash)
    return 0;

  for ( i = 0; i < nloci; i ++)
    {
      if(hashtable_count(group1->intmaps[i]) != hashtable_count(group2->intmaps[i]))
	return 0;

      itr = hashtable_iterator((*group1).intmaps[i]);

      do{
	       
	key = hashtable_iterator_key(itr);
	v = hashtable_iterator_value(itr);
	int *k2, *v2;
	if(NULL == (v2 = hashtable_search((*group2).intmaps[i], key) ))
	  {
	    free(itr);
	    return 0;
	  }
	else if(*v2 != *v)
	  {
	    free(itr);
	    return 0;
	  }
					 
      } while (hashtable_iterator_advance(itr));

      free(itr);
      for(j =0; j < MAX_POSSIBILITIES; j++)
	{
	  if((*group1).possibilitysets[i][j] != (*group2).possibilitysets[i][j])
	    {
	      return 0;
	    }
	}
			       
    }


  return 1;
}

/******************************
calculates hash for a sibgroup
*******************************/
static unsigned int calculateHash(sibgroup * group)
{
  struct hashtable_iterator * itr;
  int i,j,k;
  int*key, *v;


  unsigned int hash = UINT_MAX;
  unsigned int maphash= INT_MAX;
  unsigned int phash = INT_MAX;

  

  for ( i = 0; i < nloci; i ++)
    {
      itr = hashtable_iterator((*group).intmaps[i]);
      do{
	       
	key = hashtable_iterator_key(itr);
	v = hashtable_iterator_value(itr);
	maphash ^= *key;
	//maphash <<= 2;
					 
      } while (hashtable_iterator_advance(itr));
      maphash <<= 2;
      free(itr);
			       
  }
  hash ^= maphash;
  hash <<= 8;
    for(i = 0; i < nloci; i++)
    {
      for(j = 0; j < MAX_POSSIBILITIES; j++)
	{
	  if((*group).possibilitysets[i][j] )
	    {
	      phash ^= j;
	    }
	}      
      phash <<= 2;
    }
    hash |= phash;
  (*group).hash = hash;
  return hash;
  
}


/***********************
returns the hash value associated with the sibgroup
*************************/
unsigned int sibgroupHash(void * group)
{
  // return calculateHash(group);
    return (*(sibgroup *)group).hash;
  //  return hash;
}

/**************************
initializes globals
***************************/
void init()
{
  possibilitiesMap = (struct hashtable *) create_hashtable(10, getPairInt, pairEquals ); 
  possibilities = (list *) calloc(MAX_POSSIBILITIES, sizeof(list));
  alleles = (struct hashtable **) calloc(nloci, sizeof(struct hashtable *));
  int i;
  for(i = 0; i < nloci; i ++)
    {
      alleles[i] = (struct hashtable *) create_hashtable(10, getIntHash, equals);
    }
  
  costs = calloc(6, sizeof(double));
  costs [NULL_ALLELE_ERROR]= 0.1;
  costs[HOMO_MISTYPE_ERROR]= 0.3;
  costs[HETERO_MISTYPE_ERROR] =0.15;
  costs[POSSIBILITY_MISFIT]= 0.4;
  maxedit = 0.5;
  lociorder = NULL;
  populateMoveMap( );
}

/***************************************************
determines if the given two pairs have same values
 *************************************************/
int pairEquals(Pair * p1, Pair * p2)
{
  if((*p1).x == (*p2).x && (*p1).y == (*p2).y)
    {
      return 1;
    }
  return 0;
}

/**************************************************
 * Determines if two possibility sets are equal
 *************************************************/
int equalsPossibilitySets(int * set1, int * set2)
{
  int i = 0;
  for(i = 0; i < MAX_POSSIBILITIES; i ++)
    {
      if(set1[i] != set2[i])
	{
	  return 0;
	}
    }
  return 1;
}

/***********************************************************
 * integer returns integer value of pair for hash purposes
 ***********************************************************/
int getPairInt(Pair * pair)
{
  int x = 0;
  x = (*pair).x;
  x *= 100;
  x+= (*pair).y;
  return x;
}

/***********************

*************************/
void cleanUp()
{
	// need to write this to clean up the data etc
  struct hashtable_iterator * itr = hashtable_iterator(sibgroupsmap);
  do{
    sibgroup * group = hashtable_iterator_key(itr);
    deleteSibgroupShallow(group, 1);
  }while(hashtable_iterator_advance(itr));
  free(itr);
  hashtable_destroy(sibgroupsmap, 0);
  sibgroupsmap = NULL;
  
}


/************************************************************************************
 adds a new item to a given doubly linked list using the 'node' data structure
 which must be deallocated later

 params:
 addTo: pointer to the list to which the item will be added
 item: pointer to the item that is to be added to the list
********************************************************************************/
void addToList(list * addTo, void * item)
{
  node * newnode = malloc(sizeof(node));
  (*newnode).next = NULL; //(*addTo).head;
  
  node * current;
  current = addTo->head;
  while(current != NULL && current->next != NULL) current = current->next;

  newnode->previous = current;
  if((*addTo).head == NULL)
  {
    addTo->head = newnode; 
  }else{
    current->next = newnode;
    

    //(*(*addTo).head).previous = newnode;
  }
  //  (*addTo).head = newnode;
  (*newnode).data = item;
  addTo->count ++;
}
/***************************************
 ***************************************/
void addToSortedList(list * addTo, int x)
{
  node * newnode = malloc(sizeof(node));
  (*newnode).next = NULL; //(*addTo).head;
  
  node * current;
  current = addTo->head;
  while(current != NULL && ((int) current->data)<x && current->next != NULL) current = current->next;
  if(current != NULL && current->data == x)
    return;
  //  newnode->next = newnode->previous = current;
  
  if(( (*addTo).head == NULL || current->previous == NULL ) && ( current == NULL ||current->data > x))
  {
    addTo->head = newnode;
    if(current != NULL)
      {
	current->previous = newnode;
      }
    newnode->previous = NULL;
    newnode->next = current;
  }
  else if(current->data >= x)
    {
    
      newnode->previous = current->previous;
      current->previous = newnode;
      //    if(newnode->previous)
	newnode->previous->next= newnode;
      newnode->next = current;      
    }
  else
    {
      newnode->previous = current;
      newnode->next = NULL;
      current->next = newnode;
    }

  //  (*addTo).head = newnode;
  (*newnode).data =(void *) x;
  addTo->count ++;
}

/*****************************************************
 * Searches List L for integer x
******************************************************/
int searchList(list * l, int x)
{
  int y= 0;
  node * p = l->current;
  if(l->head == NULL)
    return 0;
  if(p == NULL)
    p = l->current = l->head;
  if(p->data == x)
    {
      l->current = p;
       return 1;
    }
  
  if(x > p->data)
     {
       
       
       while(p != NULL && x >= p->data)
	 {
	  if( ((int)p->data) == x)
	    {
	      l->current = p;	
	      return 1;
	    }
	  p = p->next;
	  
	  
	}
    }
  else
    {
            
      while(p != NULL && x <= p->data)
	{
	  if( ((int)p ->data) == x)
	    {
	      l->current = p;
	      return 1;	
	    }
	  p = p->previous;
	  
	  
	}
    }
  return y;
}

/***************************************************************
 reads possibilites table from possibilities file
 **************************************************************/
void readPossibilities(const char * filename)
{
  if(dolog)
    printf("******************* READING POSSIBILITIES **********************\n");
  FILE * infile;
  char delims[] =  "\t, /\n\r";
  char temp[512];
  char * buffer = malloc(sizeof(char) * 512);
  int i = 0;
  int count = 0;
  char * setname, *allele1, * allele2, * read;


  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data\n");
      exit(-1);
    }
  else{
    if(dolog)
      {

	printf("Successfully opened file\n");
      }
  }

  char * progress;
  while(infile)
    {
      // each line
      read = fgets(temp, 512, infile);
      strcpy(buffer, (const char *)temp);
      if(dolog)
	printf("Read Line: %s\n", read);
      //		string input = buffer;
      if(read ==NULL || strlen(buffer) < 2)
	{
	  if(dolog)
	    printf("empty line -- breaking\n");
	  break;
	}
      if(strncmp(buffer, "\"Set",4) == 0)
	{
	  // new possibilities set
	  if(dolog)
	    printf("Possibilities set:\n");
	  count ++;
	  
	  //continue;
	}
      else{

	Pair * pair = malloc(sizeof(Pair));
	allele1 = strtok_r(buffer, delims, &progress);
	allele2 = strtok_r(NULL, delims, &progress);
	if(allele2 == NULL)
	  {
	    fprintf(errors, "Urgghhh.... cant do this, insufficient data\n");
	      return;
	  }
	if(dolog)
	  {
	    printf("%s-%s\n",allele1, allele2);
	  }
	
	(*pair).x = atoi(allele1);
	(*pair).y = atoi(allele2);
	  int * possibs = hashtable_search(possibilitiesMap, pair);
	  if(possibs == NULL)
	    {
	      possibs = calloc(MAX_POSSIBILITIES, sizeof(int));
	      for(i = 0; i < MAX_POSSIBILITIES; i++)
		{
		  possibs[i] = 0;
		}
	      //	      possibs[count]=1;
	      hashtable_insert(possibilitiesMap, pair, possibs);
	    }
	  addToList(& possibilities[count],pair);
	  possibs[count] = 1;
	  
	  
	  if(dolog)
	    printf("\n");
	  
      }
    }
  fclose(infile);
}

/****************************************************************
 reads data into the linked list 'data'

 params:
 filename: name of the file to read
****************************************************************/
void readData(const char * filename)
{
  individuals = 0;
  if(dolog)
    printf("****************** READING FILE ************************\n");
  FILE * infile;
  char temp[512];
  char* buffer = malloc(sizeof(char) * 512);
  char * name,*allele1, *allele2, *progress;

  int index = 0;
  int read = 0;
  data.head= NULL;
  data.count = 0;
  int counter = 0;
  char delims[] = "\t, /\n\r";

  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data\n");
      return;
    }
  else{
    if(dolog)
      printf("Successfully opened file\n");
  }
	

  while(infile)
    {
		// each juvenile
      read = fgets(temp, 512, infile);
      strcpy(buffer, (const char *) temp);
      if(dolog)
	    printf("Deciphering: %s\n", buffer);
      //		string input = buffer;
      if(read ==NULL || strlen(buffer) < 2)
	{
	  if(dolog)
	    printf("empty line -- breaking\n");
	  break;
	}
	
//Debug
printf("\n%s",buffer);
      
      if(strlen(buffer) <= 3)
	return;
	//if (buffer[0]==',')
	//	name = "";
      	//else
		name = strtok_r(buffer, delims, &progress);
	printf("%s\n",name);      
	//exit(0);

dataentry * j = malloc(sizeof(dataentry));
      strcpy(j->name, (const char*) name);
      
      (*j).loci = calloc(nloci, sizeof(Pair));
      index = 0;
      
      /* Extract remaining 					 * strings 		*/
      Pair * pair = (*j).loci;
      j->index = counter ++;
      while ( (allele1 = strtok_r(NULL, delims, &progress)) != NULL && index < nloci)
	{
	  
	  
	  allele2=strtok_r(NULL,delims, &progress);
	  if(allele2 == NULL)
	    {
	      fprintf(errors, "Urgghhh.... cant do this, insufficient data\n");
	      printf("allele2 == NULL\n");
	      return;
	    }
	  (*pair).x = atoi(allele1);
	  (*pair).y = atoi(allele2);
	  if(pair->x > pair->y)
	    {
	      int temp = pair->y;
	      pair->y = pair->x;
	      pair->x = temp;
	    }

	  int * k1 = malloc(sizeof(int));
	  *k1 = pair->x;
	  int * k2 = malloc(sizeof(int));
          *k2 = pair->y;
	  
	  int * val1 = hashtable_search(alleles[index], k1);
	  if(val1 == NULL)
	    {
	      val1 = malloc(sizeof(int));
	      *val1 = 0;
	      hashtable_insert(alleles[index], k1, val1);
	    }
	  else
	    free(k1);
	  (*val1)++;
	  int * val2 = hashtable_search(alleles[index], k1);
          if(val2 == NULL)
            {
              val2 = malloc(sizeof(int));
              *val2 = 0;
	      hashtable_insert(alleles[index],  k2, val2);
            }
	  else
	    free(k2);
          (*val2)++;

	  if(dolog)
	    printf("%s-%s\t",allele1, allele2);
	  pair ++;
	  index ++;
	  
	}
      if(index < nloci)
	{
	  fprintf(errors,  "Urgghhh.... cant do this, insufficient data");
	  printf("index < nloci\n");
	  return;
	}
      if(dolog)
	printf("\n");
      //	data.push_back(j);
      if(mothersmode && (strncmp(j->name, "Mother", 6)==0 || strncmp(j->name, "mother", 6)==0 ) )
	mother = j;
      else
	addToList( & data, j);

      individuals ++;
    }
  fclose(infile);
  free(buffer);
}

void readSelectedData(const char * filename, int droplocus)
{
  individuals = 0;
  if(dolog)
    printf("****************** READING FILE ************************\n");
  FILE * infile;
  char temp[512];
  char* buffer = malloc(sizeof(char) * 512);
  char * name,*allele1, *allele2, *progress;

  int index = 0;
  int assignedindex = 0;
  int read = 0;
  data.head= NULL;
  data.count = 0;
  int counter = 0;
  char delims[] = "\t, /\n\r";

  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data\n");
      return;
    }
  else{
    if(dolog)
      printf("Successfully opened file\n");
  }
	

  while(infile)
    {
		// each juvenile
      read = fgets(temp, 512, infile);
      strcpy(buffer, (const char *) temp);
      if(dolog)
	    printf("Deciphering: %s\n", read);
      //		string input = buffer;
      if(read ==NULL || strlen(buffer) < 2)
	{
	  if(dolog)
	    printf("empty line -- breaking\n");
	  break;
	}

      
      if(strlen(buffer) <= 3)
	return;
      name = strtok_r(buffer, delims, &progress);
      dataentry * j = malloc(sizeof(dataentry));
      strcpy(j->name, (const char*) name);
      
      (*j).loci = calloc(nloci, sizeof(Pair));
      index = 0;
      assignedindex=0;
      
      /* Extract remaining 					 * strings 		*/
      Pair * pair = (*j).loci;
      j->index = counter ++;
      while ( (allele1 = strtok_r(NULL, delims, &progress)) != NULL && assignedindex < nloci && index < nloci+1)
	{


	  
	  allele2=strtok_r(NULL,delims, &progress);
	  if(allele2 == NULL)
	    {
	      fprintf(errors, "Urgghhh.... cant do this, insufficient data\n");
	      return;
	    }
	  if(index == droplocus)
	    {
	      index ++;
	      continue;
	    }
	  (*pair).x = atoi(allele1);
	  (*pair).y = atoi(allele2);
	  if(pair->x > pair->y)
	    {
	      int temp = pair->y;
	      pair->y = pair->x;
	      pair->x = temp;
	    }

	  int * k1 = malloc(sizeof(int));
	  *k1 = pair->x;
	  int * k2 = malloc(sizeof(int));
          *k2 = pair->y;
	  
	  int * val1 = hashtable_search(alleles[assignedindex], k1);
	  if(val1 == NULL)
	    {
	      val1 = malloc(sizeof(int));
	      *val1 = 0;
	      hashtable_insert(alleles[assignedindex], k1, val1);
	    }
	  else
	    free(k1);
	  (*val1)++;
	  int * val2 = hashtable_search(alleles[assignedindex], k1);
          if(val2 == NULL)
            {
              val2 = malloc(sizeof(int));
              *val2 = 0;
	      hashtable_insert(alleles[assignedindex],  k2, val2);
            }
	  else
	    free(k2);
          (*val2)++;

	  if(dolog)
	    printf("%s-%s\t",allele1, allele2);
	  pair ++;
	  index ++;
	  assignedindex ++;	  
	}
      if(assignedindex < nloci)
	{
	  fprintf(errors,  "Urgghhh.... cant do this, insufficient data");
	  return;
	}
      if(dolog)
	printf("\n");
      //	data.push_back(j);
      if(mothersmode && (strncmp(j->name, "Mother", 6)==0 || strncmp(j->name, "mother", 6)==0 ) )
	mother = j;
      else
	addToList( & data, j);

      individuals ++;
    }
  fclose(infile);
  free(buffer);
}




// equality operator
int equals(int* x, int *y)
{
	return *x == *y;
}

/*********************
returns the value of the int as hash for hash table

params: x the value which is simply returned
**********************/
 unsigned int getIntHash(int * x)
{
	return 5000- *x;
}

/**************************************************************
  returns possibilities table entries with the given pair
 *************************************************************/
int * lookupPossibilities(Pair * p)
{
 
  int * vals = hashtable_search(possibilitiesMap, p);
  if(vals == NULL)
    vals = EMPTY_POSSIBILITIES;
  return vals;
}


/****************************************************************
Initializes a sibgroup

params:
group: pointer to the sibgroup which will be initialized
***************************************************************/
void initializeSibgroup(sibgroup * group)
{

	int i,j,k;
	(*group).possibilitysets = calloc(nloci, sizeof(int *));
	(*group).allelecounts = calloc(nloci, sizeof(int));	
	(*group).intmaps = calloc(nloci, sizeof( struct hashtable *));
	group->edit = 0;
	group->mark = 0;
	group->mutate = 0;
	for(i = 0; i < nloci; i ++)
	{
	  (*group).allelecounts[i] = 1;
	  ((*group).intmaps)[i] = newAlleleMap(i);
	  (*group).possibilitysets[i] = calloc(MAX_POSSIBILITIES, sizeof(int));
	  for(j = 0; j < MAX_POSSIBILITIES; j++)
	    {
	      (*group).possibilitysets[i][j]= 1;
	    }
	}

	(*group).assignments = malloc(sizeof(list));//calloc(individuals, sizeof(int ));
	group->assignments->head = 0x0;	
	group->assignments->current = 0x0;
	group->assignments->count =0;
	group->founder.x =0;
	group->founder.y = 0;
	(*group).done = 0;
}


/****************************************************************************
* computes the intersection of two sets of possibilities in form of pointers
* to integers arrays of size MAX_POSSIBILITIES
*
* params:
* set1: first possibilities set
* set2: second possibilities set
**************************************************************************/
int * intersectPossibilities(int* set1, int * set2)
{
  if(set1 == NULL || set2 == NULL)
    {
      printf("ERROR: Trying to intersect an empty possibilities set \n");
      exit(-1);
      //return NULL;
    }
	int * result = calloc(MAX_POSSIBILITIES, sizeof(int));
	int i = 0;
	for(i = 0; i < MAX_POSSIBILITIES; i++)
	{
	  if((set1[i]!= 0 && set1[i]!= 1) ||( set2[i] !=0 && set2[i] != 1))
	  {
	    printf("Illegal set possibility values\n");
	    exit(-1);

	  }
		if(set1[i] && set2[i])
			result[i] = 1;
		else
		  result[i]= 0;
	}
	return result;
}

/**************************************************************
 copies integer values from one integer array to another, 
	both size MAX_POSSIBILITIES
***************************************************************/
void copyPossibilities(int * target, const int * source)
{
	int i;
	for(i = 0; i < MAX_POSSIBILITIES; i++)
	{
		target[i] = source[i];
	}
}

/**************************************************************
 creates a deep copy of a given sibgroup and returns it
***************************************************************/
sibgroup* copySibgroup(sibgroup * source)
{
  if(source->mutate == 293)
    printf(" " );

  sibgroup* group = (sibgroup*) malloc(sizeof(sibgroup));
  int i,j,k,*key,*v;
  group->mark = 0;
  struct hashtable_iterator * itr;
  (*group).possibilitysets = calloc(nloci, sizeof(int *));
  (*group).allelecounts = calloc(nloci, sizeof(int));
  (*group).founder = source->founder;
  (*group).intmaps = calloc(nloci, sizeof( struct hashtable *));
  (*source).mutate ++;
  group->mutation = 1;
  group->mutate = source->mutate;
  group->done = 0;
  (*group).mutationstamp = source->mutate;
  for(i = 0; i < nloci; i ++)
    {
      
      ((*group).intmaps)[i] = (struct hashtable *) create_hashtable(6, getIntHash, equals );
      ((*group).allelecounts)[i] = (*source).allelecounts[i];
     
      if (hashtable_count((*source).intmaps[i]) > 0)
	{
	  itr = hashtable_iterator((*source).intmaps[i]);
	  do {
            key = hashtable_iterator_key(itr);
            v = hashtable_iterator_value(itr);
	    int * k2 = malloc(sizeof(int));
	    *k2 = *key;
	    int * v2 = malloc(sizeof(int));
	    *v2 = *v;

	    hashtable_insert((*group).intmaps[i],k2,v2) ;

	  } while (hashtable_iterator_advance(itr));
	  free(itr);	
	}
      else{
	printf("ERROR: FOUND a sibsgroup without intmapping\n");
	printf("Founders %d %d Mutation %d Mutation Stamp %d\n", source->founder.x,source->founder.y, source->mutate, source->mutationstamp);
	exit(-1);
      }
   
      
      (*group).possibilitysets[i] = calloc( MAX_POSSIBILITIES, sizeof(int) );
      for(j = 0; j < MAX_POSSIBILITIES; j++)
	{
	  (*group).possibilitysets[i][j]= (*source).possibilitysets[i][j];
	}
    }
  
  (*group).assignments = malloc(sizeof(list));//calloc(individuals, sizeof(int));
  copyList(group->assignments, source->assignments);
  

  return group;
}

 void blank(void *k){}

void deleteSibgroup(sibgroup * group)
{
  deleteSibgroupShallow(group, 0);
}

/*************************************************************
 Frees memory used by a sibgroup
************************************************************/
void deleteSibgroupShallow(sibgroup * group, int shallow)
{
  if(group->founder.x == 1 && group->founder.y == 40 && group->mutate>= 58)
    {
      if(dolog)
	printf("deleting a suspect herre\n");
    }

  int i;
  //free
  for( i = 0; i < nloci; i ++)
    {
       //free((*group).intmaps[i]);
      /*struct hashtable_iterator * itr = hashtable_iterator(group->intmaps[i]);
            do{
	int * k = hashtable_iterator_key(itr);
	int * v = hashtable_iterator_value(itr);
	//	hashtable_remove(group->intmaps[i], k);
	//	free(k);
	free(v);
      }while(hashtable_iterator_advance(itr));*/
	hashtable_destroy((*group).intmaps[i], 1);
	free((*group).possibilitysets[i]);
	
    }
  free((*group).possibilitysets);
  free((*group).intmaps);
  deleteList(group->assignments, blank);
  free((*group).assignments);
  free((*group).allelecounts);

  if(!shallow)
    free(group);
}

void datacleaner(void * p)
{
  dataentry * e = p;
  free(e->loci);
  free(e);
}

/**********************
 deletes a given list with and using the given 'cleaner' function to free up data

*******************************************/
void deleteList(list * target, void * cleaner() )
{
  node * current = (*target).head;
  while(current != NULL)
    {
      node * next = (*current).next;
      cleaner((*current).data);
      free(current);
	  current = next;
    }
}

/*******************************************
 newAlleleMap
 builder function for allelemaps, takes care of wild card mapping
**************************************************/
struct hashtable * newAlleleMap(int loc )
{
  struct hashtable * map = (struct hashtable *) create_hashtable(6, getIntHash, equals );  
  int * key = malloc(sizeof(int));
  int * value = malloc(sizeof(int));
  *key = -1;
  *value = 9;
  hashtable_insert(map, key, value);

  if(mothersmode){
    if(mother->loci[loc].x == mother->loci[loc].y)
      {
	int * key1 = malloc(sizeof(int));
	int * val1 = malloc(sizeof(int));
	*key = mother->loci[loc].x;
	*val1 = 1;
	hashtable_insert(map, key1, val1);
      }
    else{
      int * key1 = malloc(sizeof(int));
      int * val1 = malloc(sizeof(int));
      int * key2 = malloc(sizeof(int));
      int * val2 = malloc(sizeof(int));
      *key1 = mother->loci[loc].x;
      *key2 = mother->loci[loc].y;
      *val1 = 1;
      *val2 = 2;
      hashtable_insert(map, key1, val1);
      hashtable_insert(map, key2, val2);
    }
  }

  return map;
}


/*********************
 checks if the given array of integers is all zeros
 *********************/
int isEmptyIntersection(int * intersection)
{
  if(intersection == NULL)
    {
      printf("Checking emptiness of empty intersection \n");
      exit(-1);
      //    return 1;
    }
  int i;
  for(i = 0; i < MAX_POSSIBILITIES; i++)
    {
      if(intersection[i])
	return 0;
    }
  return 1;
}

/****************************************************
 Assigns an Individual to Sibgroup

returns 0 if assignment was not possible, 1 if assignment is
possible without any change to the existing alleles of group
2 if it requires a change

the given group IS modified, therefore a copy may be made 
prior to calling this function
******************************************************/
int assignIndividualToGroup(sibgroup * group, individual * indi)
{
  int locus = 0;
  int loc = 0;
  struct hashtable * allelemappings;
  int * possibilities;
  int * mappedvalue = NULL;
  //sibgroup * group = copySibgroup(sourcegroup);
  int failed = 0;
  int modified = 0;

  for (loc = 0 ; loc < nloci && failed == 0; loc ++)
    {
      locus = lociorder[loc];
      allelemappings = (*group).intmaps[locus];
      possibilities = (*group).possibilitysets[locus];
      Pair mappedpair;
      
      if(NULL == (mappedvalue = hashtable_search(allelemappings, & (*indi).loci[locus].x)))
	{
	  modified = 1;
	  
	  if((*group).allelecounts[locus] < MAX_ALLELES_PER_LOCUS)
	    {
	      mappedpair.x = adjustAlleles(group, locus, indi->loci[locus].x);
	      
	      /*
	      int * key = malloc(sizeof(int));
	      *key = (*indi).loci[locus].x;
	      int * val = malloc(sizeof(int));
	      (*group).allelecounts[locus] ;
	      *val = (*group).allelecounts[locus]++ ;
	      mappedpair.x = *val;
	      hashtable_insert(allelemappings, key, val);
	      */
	      
	    }
	  else{
	    failed = 1;
	    break;
	  }
	}else
	{
	  mappedpair.x = *mappedvalue;
	}

      if(NULL == (mappedvalue = hashtable_search(allelemappings, & (*indi).loci[locus].y)))
	{
	  modified = 1;
	  
	  if((*group).allelecounts[locus] < MAX_ALLELES_PER_LOCUS)
	    {
	      mappedpair.y = adjustAlleles(group, locus, indi->loci[locus].y);
	      /*
	      int * key = malloc(sizeof(int));
	      *key = (*indi).loci[locus].y;
	      int * val = malloc(sizeof(int));
	      (*group).allelecounts[locus] ;
	      *val = (*group).allelecounts[locus] ++;
	      mappedpair.y = *val;
	      hashtable_insert(allelemappings, key, val);
	      */
	    }
	  else{
	    failed = 1;
	    break;
	  }
	}else{
	mappedpair.y = * mappedvalue;
      }
	  // possibilities
      if(!failed)
	{
	  int * lookeduppossibilities = lookupPossibilities(& mappedpair);
	  if(lookeduppossibilities == NULL)
	    {
	      lookeduppossibilities = EMPTY_POSSIBILITIES;//calloc(MAX_POSSIBILITIES, sizeof(int));
	    }
	  int * intersection = intersectPossibilities((*group).possibilitysets[locus], lookeduppossibilities);
	  if(mothersmode)
	    {
	      int * intersection1 = intersectPossibilities(motherpossibsmap[loc], intersection);
	      free(intersection);
	      intersection = intersection1;
	    }
	  if(isEmptyIntersection(intersection))
	    {
	      free(intersection);
	  failed = 1;
	  break;
	    }
	  else if(!equalsPossibilitySets(intersection, group->possibilitysets[locus]))
	{
	  modified = 1;
	  free((*group).possibilitysets[locus]);
	  
	  (*group).possibilitysets[locus] = intersection;
	}
	  else
	    {
	      free(intersection);
	    }
	}
    }	
  
  
      
    
  if(failed)
    {
      return 0;
    }
  else if(modified == 1)
    {
      return 2;
    }
  else
    {
      return 1;
    }
}

/******************************************************
   Writes results to file in a tab separated format
 ****************************************************/
void writeResultsFile(const char * filename)
{
  FILE * file = dumpfile;
  FILE * logfile;
  /*  if( NULL == (file = fopen( filename, "w")))
    {
      printf("Unable to write results\n");
      exit(-1);
      }*/
  if(dolog)
    {
      printf("Opened results file for writing");
      logfile= fopen("siblings.log", "w");
    }
  struct hashtable_iterator * itr;

  itr = hashtable_iterator(sibgroupsmap);
  int groupcounter = dumpgroupcounter;
  do{
    sibgroup * group =hashtable_iterator_key(itr);
    int i = 0;
    group->mark = groupcounter;
    fprintf(file, "\"Set(%d):\"", groupcounter ++);

    //    node * p = group->assignments->head;
    for(i = 0; i < individuals; i ++)
      {
	//	if(p && ((int) p->data) == i)
	if(searchList(group->assignments, i))
	  //	    p = p->next;
	  {
	    fprintf(file, "\t%d", 1);

	  }
	else
	  fprintf(file, "\t%d", 0);
      }
    if(dolog)
      logGroup(logfile, group);
    fprintf(file, "\n");
  }while(hashtable_iterator_advance(itr));


  free(itr);
  int i,j;

  /// now add singletons

  for(i = 0; i < individuals; i ++)
    {
      int temp = groupcounter;
      fprintf(file, "\"Set(%d):\"", groupcounter ++); 
      for(j = 0; j <individuals; j ++)
	{
	  if(i != j)
	    fprintf(file, "\t0");
	  else
	    fprintf(file, "\t1");
	}
      fprintf(file, "\n");
    }

}

/***********************************************
 * performs initialization needed for min fathers.
 ***********************************************/
void initMother()
{
  mothersmode = 1;
  int i=0;
  motherpossibsmap = calloc(nloci, sizeof(int*));
  for(i = 0; i < nloci; i ++)
    {
      motherpossibsmap[i] = calloc(MAX_POSSIBILITIES, sizeof(int));
      if(mother->loci[i].x == mother->loci[i].y)
	{
	  motherpossibsmap[i][8] = 1;
	  motherpossibsmap[i][9]=1;
	  motherpossibsmap[i][10]=1;
	  motherpossibsmap[i][12]=1;
	}
      else{
	motherpossibsmap[i][2] = 1;
	motherpossibsmap[i][4] = 1; 
	motherpossibsmap[i][5] = 1; 
	motherpossibsmap[i][7] = 1; 
	motherpossibsmap[i][9] = 1;  
	motherpossibsmap[i][13] = 1; 
	motherpossibsmap[i][14] = 1; 
      }
    }
}


void writeGamsFile(const char * filename, struct hashtable * sets)
{
  FILE * fid = fopen( filename, "w");
  if(fid == NULL)
    {
      printf("ERROR: UNABLE TO WRITE GAMS FILE\n");
      exit(-1);
    }

  //[m,n] = size(adat);
  //outname = strcat(inname,".gms");
  //fid = fpen(outname,"w");
  int n = individuals;
  int m = hashtable_count(sets) + individuals;
  int i,j;
  fprintf(fid, "$ONEMPTY \n \n");
  fprintf(fid, "SET i   sibling   /s1*s%d/; \n",n);
  fprintf(fid, "SET j   sibset    /t1*t%d/; \n \n",m);
  fprintf(fid, "TABLE A(j,i)    sets covered by node i \n");
  fprintf(fid, "     ");
  for(i=1; i <= n; i++)
	 fprintf(fid, "\t s%d",i);
  fprintf(fid, " \n");
  struct hashtable_iterator * itr;
  itr = hashtable_iterator(sets);
  for(i = 1;  ; i++)
    {
      sibgroup * group = hashtable_iterator_key(itr);
      fprintf(fid, "t%d",i);
      for ( j = 1; j <= n; j ++)
	{
	  int assignment = 0;
	  if(searchList(group->assignments, j -1))
	    assignment = 1;
	  fprintf(fid, "\t %d",assignment);
	}
      fprintf(fid, " \n");
      if(i >= hashtable_count(sets)) break;
      hashtable_iterator_advance(itr);
    }
  i++;
  free(itr);
  for(; i <=m ; i ++)
    {
      //      fprintf(fid, "\"Set(%d):\"", groupcounter ++); 
      //      sibgroup * group = hashtable_iterator_key(itr);
      //	 fprintf(fid, "t%d",i);
      fprintf(fid, "t%d",i);
      for ( j = 0; j < n; j ++)
	{
	     int assignment = 0;
	     if(i-  hashtable_count(sets)-1 == j)
	       assignment = 1;
	     fprintf(fid, "\t %d",assignment);
	}
      fprintf(fid, " \n");
 

    }
    fprintf(fid, "; \n \n");
    fprintf(fid, "$OFFLISTING \n");
    fprintf(fid, "***** Lst File Options ***** \n");
    fprintf(fid, "OPTION solprint=off; \n");
    fprintf(fid, "OPTION sysout=off; \n");
    fprintf(fid, "OPTION limrow=0; \n");
    fprintf(fid, "OPTION limcol=0; \n \n");
    fprintf(fid, "BINARY VARIABLES \n");
    fprintf(fid, "        xset(j)    sibling j \n");
    fprintf(fid, "; \n");
    fprintf(fid, "EQUATIONS \n");
    fprintf(fid, "        setcov(i) \n");
    fprintf(fid, "	      objective \n");
    fprintf(fid, "; \n");
    fprintf(fid, "VARIABLE TF; \n");
    fprintf(fid, " \n");
    fprintf(fid, "setcov(i).. \n");
    fprintf(fid, "sum(j, A(j,i)*xset(j)) =g= 1; \n");
    fprintf(fid, " \n");
    fprintf(fid, "* Objective Function \n");
    fprintf(fid, "objective.. \n");
    fprintf(fid, "TF =e= sum(j, xset(j)); \n");
    fprintf(fid, " \n");
    fprintf(fid, "***** Model Definition ****** \n");
    fprintf(fid, "MODEL setcover / ALL /; \n");
    fprintf(fid, "***** Solution Options ******* \n");
    fprintf(fid, "OPTION lp=cplex; \n");
    fprintf(fid, "OPTION mip=cplex; \n");
    fprintf(fid, "OPTION optcr=0.01; \n");
    fprintf(fid, "OPTION optca=0.1; \n");
    fprintf(fid, "OPTION iterlim=5000; \n");
    fprintf(fid, "OPTION reslim=50; \n");
    fprintf(fid, "option rmip = cplex; \n");
    fprintf(fid, "SOLVE setcover using mip minimizing TF; \n \n");
    fprintf(fid, "DISPLAY xset.l; \n");
    fclose(fid);
    //    output = 0;
}


void deleteItem(list * ll, void * item, void * freer(void *))
{
  node * n;
  n = ll->head;
  while( n != NULL && n->data != item)
    {
      n = n ->next;
    }
  if (n == NULL)
    {
      printf("ERROR: TRYING TO DELETE NONEXISTENT ITEM\n");
      exit(-1);
    }
  if(n == ll->head)
    {
      node * next = n->next;
      freer(n->data);
      free(n);
      ll->head = n;
    }
  else
    {
      node * next = n->next;
      node * prev = n->previous;
      if( n->next != NULL)
	{
	  n->next->previous = prev;
       
	}
      if(n->previous != NULL)
	{
	  n->previous->next = next;
	}
      freer(n->data);
      free(n);
    }
}
