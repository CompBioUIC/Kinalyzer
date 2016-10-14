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
#include <time.h>
#include <math.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "hashtable.h"
#include "hashtable_itr.h"
#include "structures.h"
#include "main.h"
#include "utilities.h"

#define errors stderr
#define dumping 0
struct hashtable* intersectsibgroupsIntList(list * currentList, list * sibgroupslist);
long blocksperthread;

int dumpgroupcounter = 0;
FILE * dumpfile = NULL;
int ** pmovemap;
double totaldifftime = 0.0;
int intersects=0;
int EMPTY_POSSIBILITIES[25]={0};
int blocks = 0;

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

  // APR - THIS IS AWESOME, COMPARING AN INT TO A VOID*!!! ugghh...
  if((int)p->data == x)
    {
      l->current = p;
       return 1;
    }
  
  // APR - THIS IS AWESOME, COMPARING AN INT TO A VOID*!!! ugghh...
  if(x > (int)p->data)
     {
       // APR - THIS IS AWESOME, COMPARING AN INT TO A VOID*!!! ugghh...
       while(p != NULL && x >= (int)p->data)
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
      // APR - THIS IS AWESOME, COMPARING AN INT TO A VOID*!!! ugghh...      
      while(p != NULL && x <= (int)p->data)
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


void deleteItemByNode(list * ll, node* n, void freer(void *))
{
  /*  node * n;
  n = ll->head;
  while( n != NULL && n->data != item)
    {
      n = n ->next;
      }*/
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
      n=NULL;
      ll->head = next;
      next->previous=NULL;
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
  ll->count --;
}


void printIntList(list * intlist)
{
  node * current = intlist->head;
  printf("Dumping:");
  for(; current != NULL; current = current->next)
    {
      printf (" %d", (int)current->data);
      
    }
  printf("\n");
}

list * newList()
{
  list * l = (list *) malloc(sizeof(list));
  l->head=l->current=NULL;
  l->count = 0;
  return l;
}

void printIntListList(list * intintlist)
{
  node *currentlist ;
  for(currentlist = intintlist->head; currentlist!= NULL; currentlist= currentlist->next)
    {
      list * intlist = (list *) currentlist->data;
      node * current = intlist->head;
      printf("Dumping:");
      for(; current != NULL; current = current->next)
	{
	  printf (" %d", (int)current->data);
      
	}
      printf("\n");
    }
}

/**************************************************
 * Reads Move Map from file
 *************************************************/
void populateMoveMap()
{
  pmovemap = (int**) calloc(MAX_ALLELES_PER_LOCUS, sizeof(int*));
  int i;
  for(i = 0; i < MAX_ALLELES_PER_LOCUS; i ++)
    {
      pmovemap[i] = (int*) calloc(MAX_POSSIBILITIES, sizeof(int));
    }
  FILE * infile = fopen( "movemap", "r");
  char *temp = (char*) malloc(sizeof(char*) * 512);
  char* read = fgets(temp, 512, infile);
  char * x, *y, *z;
  char * delims = ",;\n ";
  /*char * progress;*/
  while((read != NULL) && infile)
    {
      x = strtok/*_r*/(temp, delims/*, &progress*/);
      int i = atoi(x);

      y = strtok/*_r*/(NULL, delims/*, &progress*/);
      int j = atoi(y);

      z = strtok/*_r*/(NULL, delims/*, &progress*/);
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
  struct hashtable_itr * itr = hashtable_iterator(allelemap);
  do
    {
      key = (int *) hashtable_iterator_key(itr);
      value = (int *) hashtable_iterator_value(itr);
      if(*key < allele)
	{
	  newindex ++;
	}
    }while(hashtable_iterator_advance(itr));
  free(itr);


  int * newkey = (int *) malloc(sizeof(int));
  int * newvalue = (int * ) malloc(sizeof(int));

  *newkey = allele;
  *newvalue = newindex;

  int i = 0;
  for( i = count -1 ; i >= newindex ; i --)
  {
    int * possibs = group->possibilitysets[locus];
    itr = hashtable_iterator(allelemap);
    do{
      key = (int*)hashtable_iterator_key(itr);
      value = (int*)hashtable_iterator_value(itr);

      }while(hashtable_iterator_advance(itr) && *value != i );
    (*value)++;
    free(itr);
    int j=1;
    int * newpossibs = (int*)calloc(MAX_POSSIBILITIES, sizeof(int));
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
void dumpGroup(sibgroup * group, struct hashtable *  sibgroupsmap)
{
  struct hashtable_itr * itr = hashtable_iterator(sibgroupsmap);
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
void logGroup(FILE * logfile, sibgroup * group, int nloci)
{
  fprintf(logfile, "Set(%d)\n", group->mark);
  fprintf(logfile, "Founders x:%d, y:%d\n", group->founder.x, group->founder.y);
  int locus = 0;
  struct hashtable_itr * itr;
  
  for(locus = 0; locus < nloci; locus ++)
    {
      fprintf(logfile, "Locus: %d\nAllele Mappings:\n", locus);
      struct hashtable * allelemap = group->intmaps[locus];
      
      itr = hashtable_iterator(allelemap);
      do{
	int * key = (int*)hashtable_iterator_key(itr);
	int * value = (int*)hashtable_iterator_value(itr);
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
    {
      return;
    }
  node * newnode = (node*)malloc(sizeof(node));
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
      newnode = (node*)malloc(sizeof(node));
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
struct hashtable* generateInitialGroups(int * lociorder, int nloci)
{
  int count = 0;
  int recurrences =0;
  int i = 0;
  node * current = data.head;
  struct hashtable * sibgroupsmap = (struct hashtable *) create_hashtable(individuals * individuals, sibgroupHash, sibgroupEquals); 
  sibgroupsmap->nloci = nloci;
  for(i = 0; i < individuals; i ++, current = (*current).next )
    {
      node * current2 = (*current).next;
      int j;
      for(j = i+1; j < individuals; j++, current2 = (*current2).next)
	{
	  sibgroup * group = (sibgroup*)malloc(sizeof(sibgroup));
	  initializeSibgroup(group,nloci);

	  int assignment = assignIndividualToGroup(group,(individual*) (*current).data, lociorder, nloci);
	 
	  if(assignment == 0)
	    {
		printf("ERROR: UNABLE TO GENERATE INITIAL SINGLETON");
		exit(-1);
	    }
	  int assignment2 = assignIndividualToGroup(group, (individual*) (*current2).data, lociorder, nloci);
	  if(assignment2 == 0)
	    {
		printf("ERROR: UNABLE TO GENERATE INITIAL GROUP");
		exit(-1);
	    }
	  group->founder.x = i;	  
	  group->founder.y = j;
	  addToSortedList(group->assignments, i);
	  addToSortedList(group->assignments, j);
	  calculateHash(group, nloci);
	  sibgroup * exist;
	  if(NULL == (exist = (sibgroup*)hashtable_search(sibgroupsmap, group)))
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
	      deleteSibgroup(group,nloci);
	    }
      }
    }
  if(dolog)
    {
      printf("Done generating initial groups: %d Reccurences avoided: %d\n", count, recurrences);
    }
  return sibgroupsmap;
}

/**************************************************************
 * Sorts by Allele Frequency
 *************************************************************/
void sortLociByAlleleFrequency(unsigned int ** lociorder)
{
  int i = 0;
  int j = 0;
  *lociorder = (unsigned int*)calloc(nloci, sizeof(int));
 for( i = 0; i < nloci; i ++)
   {
     (*lociorder)[i] = i;
   }

 for( i = 0; i < nloci; i ++)
    for(j = i +1; j < nloci; j ++)
      {
	if(hashtable_count(alleles[(*lociorder)[i]]) < hashtable_count(alleles[(*lociorder)[j]]))
	  {
	  // APR - int temp? this is a swap, making int temp be unsigned int* and no longer dereferencing lociorder
	    unsigned int* temp = lociorder[i];
	    lociorder[i] = lociorder[j];
	    lociorder[j]= temp;
	  }
      }
}

/********************************************************************************
 *
 *******************************************************************************/
unsigned int getIntListHash(void * intlist_param)
{
  list* intlist = (list*) intlist_param;
  // APR - again, very fishy, why so many type errors?
  int hash = (int)intlist->head->data;
  int count = 0;
  for(intlist->current = intlist->head; intlist->current!= NULL; intlist->current = intlist->current->next)
    {
      int val = (int)intlist->current->data;
      hash ^= val;//((hash ^ val)>>(intlist->count%8)|(hash ^ val)<<(intlist->count%8));
      count ++;
    }
  //  hash ^= intlist->count;
  return hash;
}

unsigned int getArrayHash(void * x_param)
{
  unsigned int* x = (unsigned int*) x_param;
  int hash = 0;
  unsigned int * xptr = x;
  int i;
  for( i = 0; i < blocks; i ++)
    {
      hash ^=  (*xptr);
      xptr ++;
    }
  return hash;
}

int equalsArray(void* x_param, void* y_param, int dummy_unused)
{
  unsigned int * x = (unsigned int *)x_param;
  unsigned int * y = (unsigned int *)y_param;
  unsigned int * xptr = x;
  unsigned int * yptr = y;
  int i;
  for( i = 0; i < blocks; i ++)
    {
      if (*xptr != *yptr)
	{
	  return 0;
	}
      xptr ++;
      yptr ++;
    }
  return 1;
}


int equalsIntList(void * intlist1_param, void * intlist2_param, int dummy_unused)
{
  list* intlist1 = (list*) intlist1_param;
  list* intlist2 = (list*) intlist2_param;
  if(intlist1->count != intlist2->count) return 0;
  for(intlist1->current = intlist1->head, intlist2->current= intlist2->head;
      intlist1->current!= NULL;
      intlist1->current = intlist1->current->next, intlist2->current = intlist2->current->next)  
    {
      if((unsigned int)intlist1->current->data != (unsigned int)intlist2->current->data)
	return 0;
    }
  return 1;
}

void updateTable(struct hashtable * table, unsigned int *zlist, unsigned char* heap)
{
	    if( NULL == hashtable_search(table, zlist))
	      {
		int ecount = hashtable_count(table);
		if ((ecount+1)*blocks > blocksperthread)
		  { 
		    printf("ERROR: INCREASE BLOCKS PER THREAD!! %d/%d\n",ecount*blocks,blocksperthread);
		    exit(-1);
		  }
		int gap  =(sizeof(int)*ecount*blocks);
		// APR - this is horrible, gap could be multiplied by sizeof char or by sizeof int, which one?
		// am guessing it's meant to be sizeof(char) and that gap is bytes
		unsigned char * char_copy = heap+gap; //heap+(index);
		unsigned int* copy = (unsigned int*) char_copy;
		int i;
		for(i = 0; i < blocks; i ++)
		  {
		    copy[i]=zlist[i];
		  }

		hashtable_insert(table, copy,copy);

	      }
}

void checkList(list * currentList)
{
  printf("entered checklist:%x\n",currentList->head);
  node * n = currentList->head;
  while(n->next)
    {
      unsigned int * xlist = (unsigned int *)n->data;
      int i;
      for(i=0; i < blocks; i ++)
	{
	  printf("%d:%u\n", i, xlist[i]);
	}
      n = n->next;
    }
  printf("exiting checklist\n");
}

unsigned int * copyBlocks(unsigned int * data)
{
  unsigned int * res = (unsigned int*)calloc(blocks, sizeof(unsigned int));
  int i ;
  for ( i = 0; i < blocks; i ++)
    {
      res[i] = data[i];
    }
  return res;
}
int thr_id=0;
//#pragma omp threadprivate(thr_id)
int maxthreads;



list * cleanAndSerialize(struct hashtable * table)
{
  list * result = (list *)malloc(sizeof(list));
  result->head = NULL;
  result->count = 0;
  result->current = NULL;
    unsigned int * grp,*cpgrp;

  struct hashtable_itr * itr;
  itr = hashtable_iterator(table );
  
  do
    {
      grp=(unsigned int*) hashtable_iterator_key(itr); //(localheaps[i]+(blocks*j));
      cpgrp = copyBlocks(grp);
      //     cpgrp[0];
      addToList(result, cpgrp);
      // *** TODO      
    }while(hashtable_iterator_advance(itr));
    cleanUpOverlaps(result, 0);
  return result;
}

/********************************************************
 ********************************************************/
struct hashtable* intersectsibgroups(list * currentList,list * sibgroupslist)
{
  printf("currentList: %x, sibgroupslist: %x\n",currentList, sibgroupslist);
  int nthr=1; 

  
  if(currentList == NULL|| sibgroupslist == NULL)
    {
      printf("ERROR: null value in intersectsibgroups\n");
      exit(-1);
    }

  blocksperthread = ((long)sibgroupslist->count*currentList->count*blocks);
  if(maxthreads > 3 && blocksperthread >500)
    {
      blocksperthread = ceil(blocksperthread / ((double)maxthreads/2));
    }
  else{
    //    blocksperthread = ceil(blocksperthread / 2.0);
  }
 
  struct hashtable * intersection;
  intersection = (struct hashtable *) create_hashtable((sibgroupslist->count*currentList->count), getArrayHash, equalsArray); 
  intersection->nloci = nloci;
  //  int heapsize = sibgroupslist->count*currentList->count*20*blocks;
  //  unsigned int ** heap = calloc( heapsize, sizeof(unsigned int));
  int counter = 0;

    /*  for(currentList->current = currentList->head; currentList->current != NULL;
	currentList->current = currentList->current->next)*/
  int nthr_checked=0;
    printf("Intersecting %d by %d\n", currentList->count, sibgroupslist->count);
  unsigned int ** ptrs = (unsigned int **)calloc(currentList->count, sizeof(int *));
  int ct = 0;
  node * n;
  unsigned int ** ptrs2 = (unsigned int **)calloc(sibgroupslist->count, sizeof(int*));
  
  for(ct = 0, n = currentList->head; ct < currentList->count  ; ct++, n = n->next) 
     { 
       ptrs[ct]=(unsigned int*)n->data; 
       //       *ptrs[ct];
     } 
  for(ct = 0, n = sibgroupslist->head; ct < sibgroupslist->count  ; ct++, n = n->next) 
     { 
       ptrs2[ct]=(unsigned int*)n->data; 
     } 

  //  printf("created a ptrs array of size %d \n", ct);
  const unsigned int * xlist;
  unsigned int *ylist, empty;
  unsigned int zlist[60];

  //  int i;
  //  int thr_id=0;
  int i,counter2,startindex,sgcount,counter3;
  sgcount = sibgroupslist->count;
  struct hashtable ** localtables;
  unsigned int ** localheaps;
  localtables = (struct hashtable ** )calloc(maxthreads,sizeof(struct hashtable *));

  localheaps = (unsigned int **)calloc(maxthreads,sizeof(unsigned int *));
  for(counter = 0; counter < maxthreads; counter++)
    {
      //      localheaps[counter]=NULL;
      localheaps[counter]=(unsigned int*)calloc(blocksperthread,sizeof(unsigned int)); 
      if(localheaps[counter]== NULL)
	{
	  //  printf("Unable to allocate %d memory!!! Quitting\n", blocksperthread);
	  // exit(-1);
	  printf("Unable to allocate %d memory!! Havling blocks per thread \n", blocksperthread);
	  while(localheaps[counter] == NULL)
	    {
	      blocksperthread /= 2;
	      
	      localheaps[counter] = (unsigned int*)calloc(blocksperthread,sizeof(unsigned int)); 
	    }
	  //	  printf("Unable to allocate %d memory!!! Quitting\n", blocksperthread);
	  //exit(-1);
	}
    }

  for(counter = 0; counter < maxthreads; counter++)
    {
      //      localtables[counter]=NULL;
      localtables[counter]= (struct hashtable *) create_hashtable(blocksperthread/sizeof(unsigned int), 
								     getArrayHash, equalsArray); 
      localtables[counter]->nloci= nloci;
    }

  //  printf("before loop Local Table ptr %X",localtables);
  int clcount = currentList->count;
#ifdef _OPENMP
#pragma omp parallel for default(shared) private(thr_id,i,zlist,counter, xlist,ylist,empty,startindex,counter2,counter3,nthr) \
  shared(localheaps,localtables,blocksperthread, blocks,sibgroupslist, currentList,nthr_checked,ptrs,ptrs2,clcount,sgcount) schedule(static)
#endif // _OPENMP
  for(counter = 0; counter < clcount; counter ++) 
    { 
 #ifdef _OPENMP
 
      if(nthr_checked == 0) 
 	{ 
 	  nthr_checked =1;
	  nthr = omp_get_num_threads(); 
 
	  printf("in intersect sibgroups current threads is %d\n", nthr); 
	  
	} 
      //#endif 
      //xlist = ptrs[counter];
      nthr = omp_get_num_threads(); 
      thr_id = omp_get_thread_num();       
      // 
#endif // _OPENMP


      xlist = ptrs[counter];
      //      printf("Thread ID: %d\n", thr_id);
      startindex = (thr_id)*(sgcount/nthr);
      // printf("thread id %d, nthr %d, startindex %d, sgcount %d\n",thr_id,nthr,startindex,sgcount);
      for(counter2 = startindex, counter3 = 0; counter3 < sgcount ; counter2 = (counter2 +1)%sgcount, counter3++)
	{
	  //	  printf("Thread id:%d, counter2 : %d,counter:%d\n", thr_id, counter2,counter);
	  ylist = (unsigned int*)ptrs2[counter2];//sgcurrent->data;

	  empty = 1;
	  {
	    // comparison
	    for(i = 0; i < blocks; i ++)
	      ///	    while(xlist->current != NULL && ylist->current != NULL)
	      {
		zlist[i] = xlist[i] & ylist[i];
		if(zlist[i] > 0) empty = 0;
	      }
	    
	  }

	  {
	    if(!empty)
	      {
		updateTable(localtables[thr_id], zlist, (unsigned char*)localheaps[thr_id]);     
	      }
	  }
	}// end sibgroups for loop
      
    }// end current list for loop
#ifdef _OPENMP
  #pragma omp barrier
#endif _OPENMP
  i = 0;
  
  //  printf("combining results\n");

  deleteList(currentList, blank/* free*/);

  list ** lists = (list **)calloc(maxthreads, sizeof(list*));

#ifdef _OPENMP
#pragma omp parallel for default(none) private(i) shared(localtables,lists,localheaps,maxthreads) schedule(static)
#endif // OPENMP
  for( i = 0; i < maxthreads; i ++)
   {
      // printf("i:%d\n",i);                                                                                                                                 
      if(localtables[i]!=NULL)
	{
	  //      printf("count from thread %d is %d\n", i, hashtable_count(localtables[i]));
	  if(hashtable_count(localtables[i])==0) continue;

	  lists[i]= cleanAndSerialize(localtables[i]);
	  
	  //         hashtable_destroy_leavekey(localtables[i],0);
	  
	  
	}
   }

#ifdef _OPENMP
#pragma omp barrier
#endif _OPENMP

 currentList->head=NULL;
 currentList->count = 0;
 node * tail = NULL;
 for( i = 0; i < maxthreads; i ++)
   {
     
     if(lists[i]==NULL||lists[i]->count ==0)
       {
	 printf("lists[i] empty for %d\n",i);
	 continue;
	}
     //     checkList(lists[i]);
     currentList->count += lists[i]->count;
     if(currentList->head == NULL)
       {
	 currentList->head = lists[i]->head;
	 tail = currentList->head;
       }
     else
       {
	 tail->next = lists[i]->head;
	 lists[i]->head->previous = tail;
       }
     while(tail->next) 
       {
	 tail=tail->next;
       }
   }

 //  cleanUpOverlaps(currentList, 0);
  // checkList(currentList);
 /*

  for( i = 0; i < 20; i ++)
    {
      // printf("i:%d\n",i);
      if(localtables[i]==NULL) 
	{
	  continue;
	}
      printf("count from thread %d is %d\n", i, hashtable_count(localtables[i]));
      if(hashtable_count(localtables[i])==0) continue;
      itr = hashtable_iterator(localtables[i]);
      // printf("got itr\n");
      do{
	unsigned int * grp=(unsigned int*) hashtable_iterator_key(itr);
	found = 0;
	//	printf("got key\n");
#pragma omp parallel for default(none) private(k) shared(grp, localtables,i) reduction(+:found)
	for(k = 0; k < i; k ++)
	  {
	    if(localtables[k]!=NULL && hashtable_search(localtables[k], grp)!= NULL)
	      {
		found = 1;
	      }
	  }
	if(found==0)
	  {
	    unsigned int * cpgrp = copyBlocks(grp);
	    addToList(sibgroupslist, cpgrp);
	    // hashtable_insert(intersection, cpgrp, cpgrp);
	  }
	else{
	  //	  printf("removing from table\n");
	  //	  hashtable_remove_clear(localtables[i],grp,0);
	  //	  printf("freeing grp\n");
	  //	  free(grp);
	  //	  printf("done");
	};
      }while(hashtable_iterator_advance(itr));
      //      printf("destroying table\n");
    }

  for(i = 0; i <20;i++)
    {
      //    hashtable_destroy_leavekey(localtables[i],0);
      //Z  free(localtables[i]);
      localtables[i]=NULL;
      //      printf("did table %d",i);

    }
 */
  //  printf("Combined groups %d\n", hashtable_count(intersection));
  //  hashtable_destroy_leavekey(intersection, 0);
  //  *currentList = newlist;
 
  intersects ++;
  free(localtables);
  //  walltime = time(NULL) - start;
  free(ptrs);
  free(ptrs2);
  //  free(heap); 
  for( i = 0; i < maxthreads; i ++)
    {
      // printf("i:%d\n",i);
      free(localheaps[i]);
    }
  free(localheaps);
  //  printf("\nFinished intersection.\n");
  //  printf("Matmul kernel wall clock time = %.7f sec\n", walltime);
  //  printf("Wall clock time/thread = %.7f sec\n", walltime/nthr);
	printf("%d sets in currentlist at  return\n",currentList->count);
  return intersection;
}

/*
void intersectsibgroups(list * currentList, list * sibgroupslist)
{
  struct hashtable * intersection;
  intersection = (struct hashtable *) create_hashtable(sibgroupslist->count, getIntListHash, equalsIntList); 

  list newlist;
  newlist.head=NULL;
  newlist.current = NULL;
  newlist.count = 0;

  for(currentList->current = currentList->head; currentList->current != NULL;
      currentList->current = currentList->current->next)
    {
      list * xlist = (list*) currentList->current->data;
  
      for(sibgroupslist->current = sibgroupslist->head; sibgroupslist->current != NULL;
	  sibgroupslist->current = sibgroupslist->current->next)
	{
	  
	  list * ylist = (list*)sibgroupslist->current->data;
	  
	  
	  list intersectedList;
	  intersectedList.head = NULL;
	  intersectedList.current = NULL;
	  intersectedList.count = 0;
	  {
	    // comparison
	    xlist->current = xlist->head;
	    ylist->current = ylist->head;

	    while(xlist->current != NULL && ylist->current != NULL)
	      {
		int xval = (int) xlist->current->data;
		int yval = (int) ylist->current->data;

		if(xval == yval)
		  {
		    addToSortedList(&intersectedList, xval);
		    xlist->current = xlist->current->next;
		    ylist->current = ylist->current->next;
		  }
		else if(xval < yval)
		  {
		    xlist->current = xlist->current->next;
		  }
		else if(xval > yval)
		  {
		    ylist->current = ylist->current->next;
		  }
	      }

	  }

	  if(intersectedList.count>0 && NULL == hashtable_search(intersection, &intersectedList))
	    {
	      list * copy = malloc(sizeof(list));
	      *copy = intersectedList;
	      hashtable_insert(intersection, copy, copy);
	      addToList(&newlist, copy);
	    }
	}// end sibgroups for loop
      
    }// end current list for loop

  list intersectedList;
  intersectedList.head = NULL;
  intersectedList.current = NULL;
  intersectedList.count = 0;
  deleteList(currentList, blank);
  struct hashtable_iterator * itr;
  
  //  hashtable_destroy(intersection, 0);
  *currentList = newlist;
}
*/

unsigned int * encodeList(list * source)
{
  unsigned int * p;
  p = (unsigned int *)calloc(blocks,sizeof(unsigned int));
  
  int i;
  int block = 0;
  for(i = 0; i < data.count;  block++)
    {
      int j;
      unsigned int word=0;
      for (j = 0; i < data.count && j < 8*sizeof(unsigned int); j ++,i++)
	{
	  int bit = 0;
	  if(searchList(source, i))
	    {
	      bit = 1;
	    }
	  word <<=1;
	  word |= bit;
	}
      p[block]=word;
    }
  return p;
}

/*void encodeList(list * target, list * source)
{
  target->head = NULL;
  target->count = 0;
  target->current = NULL;

  int counter =0;
  while( counter < data.count )
    {
      int index ;
      unsigned int word=0;
      for(index =0;counter < data.count && index < sizeof(unsigned int)*8; index ++, counter ++)
	{
	  int bit = 0;
	  if(searchList(source, counter))
	    {
	      bit =1;
	    }
	  word <<=1;
	  word |= bit;
	}
      addToList(target, word);
    }
    }*/

list* lociConstruct(const char * filename, int loci)
{
 
  char * val =getenv("OMP_NUM_THREADS");
  if(val)
    {
      maxthreads = atoi(val);
    }
#ifdef _OPENMP
  maxthreads =omp_get_max_threads();
#endif // _OPENMP

  int nthr=1;
  int nthr_checked = 0;
  
  
  //    #pragma omp single nowait
  
  

  int locus = 0; 
  /*  list itemsets;
  itemsets.head = NULL;
  itemsets.current=NULL;
  //  itemsets.tail = NULL;
  itemsets.count = 0;*/
  list ** itemsets = (list **)calloc(loci+1, sizeof(list*));
  list ** itemsets2 = (list **)calloc(loci+1, sizeof(list*));
  nloci=1;
  sibgroup * grp;
  struct hashtable * sibgroupsmap;
  struct hashtable_itr * itr;
  int locusorder;
  unsigned int * p;
  list * locuslist;

  blocks = ceil((double)data.count/(8*(double) sizeof(unsigned int)));
  char locusfilename[128];
#ifdef _OPENMP
#pragma omp parallel for default (none) private(locusfilename,locuslist,locusorder,p,itr,grp,locus,sibgroupsmap) shared(itemsets,loci,nthr_checked,nthr,blocks,possibilities,possibilitiesMap,alleles)
#endif // _OPENMP
  for( locus = 0; locus < loci; locus++)
    {
      
      locuslist = (list *)malloc(sizeof(list));
      locuslist->head = NULL;
      locuslist->current =NULL;
      locuslist->count = 0;
      

      /*      locuslist2 = malloc(sizeof(list));
      locuslist2->head = NULL;
      locuslist2->current =NULL;
      locuslist2->count = 0;
      */
#ifdef _OPENMP
      if(nthr_checked ==0)
	{
	  nthr_checked = 1;

	  nthr = omp_get_num_threads();
	  //nthr *=2;
	  //omp_set_num_threads(nthr);
	  printf( "\nWe are using %d thread(s)\n", nthr);
	}      
#endif      //lociorder[0]=locus;
      /** SUSPECT BUG AROUND HERE**/
      locusorder = locus;
      sibgroupsmap = reconstruct(&locusorder, 1 );
      //      addToList(& itemsets, &sibgroupsmap);
      //            do{
      // itr;
      itr = hashtable_iterator(sibgroupsmap);
      //int groupcount = 0;
      
      do{
	grp= (sibgroup*)hashtable_iterator_key(itr);
	/*
	list * p2=malloc(sizeof(list)); 
	copyList(p2, grp->assignments);
	addToList(locuslist2, p2);
	*/
	p = encodeList(grp->assignments);
	addToList( locuslist, p);
      
      }while(hashtable_iterator_advance(itr));
      //addToList(& itemsets, locuslist);
      itemsets[locus+1] = locuslist;
    cleanUpOverlaps(locuslist, 1);
      //      itemsets2[locus+1] = locuslist2;
      //      printIntListList(locuslist);
      free(itr);
      cleanUp(sibgroupsmap);

      sprintf(locusfilename,"sets_locus_%d", locus);
      //      writeResultsFileList(locusfilename, itemsets[locus+1]);
	  // }while(hashtable_iterator_
    }
 
  //  node * xyz = itemsets.head->next;
  //itemsets.current = itemsets.head;
  list current; // list of intsets
#ifdef _OPENMP
  #pragma omp barrier
#endif // _OPENMP
  current.head=NULL;
  current.count = 0;
  current.current = 0;
  list current2; // list of intsets

  current2.head=NULL;
  current2.count = 0;
  current2.current = 0;
   list all;
  all.head = NULL;
  all.current = NULL;
  all.count = 0;
  int counter;
  for(counter = 0; counter < data.count; counter ++)
    {
      addToSortedList(& all, counter);
      list *single=(list*)malloc(sizeof(list));
      single->head = NULL;
      single->count = NULL;
      single->current = NULL;
      
      addToSortedList(single, counter);
      addToList(&current2, single);
      //list * singleton = malloc(sizeof(list));
      unsigned int * sing = encodeList(single);
      addToList(&current, sing);
    }
 
  addToList(&current2, &all);
  itemsets[0]=&current;
  //  itemsets2[0]=&current2;
  unsigned int * all2;
  int step = 0;
  all2=encodeList( &all);
  loci++;
  addToList(&current, all2);
  /*  #pragma omp parallel for 
  for (counter = 1; counter < 2*loci; counter +=2)
    {
      itemsets[counter]=&current;
    }
    #pragma omp barrier*/
  printf("Now intersecting sets...\n");

  for(locus = 1; locus< loci; locus ++)
    {
      /*struct hashtable * x=*/ intersectsibgroups(itemsets[0],itemsets[locus]);
      //      struct hashtable * y = intersectsibgroupsIntList(itemsets2[0], itemsets2[locus]);
      //struct hashtable_iterator * itr = hashtable_iterator(y);

      /*      do{
	list * l = hashtable_iterator_key(itr);
	unsigned int * l2 = encodeList(l);
	if(hashtable_search(x, l2)==NULL)
	  {
	    int bugg = 1;
	  }
      }while(hashtable_iterator_advance(itr));
      */
      //      printf("Array Based: %d \t List: %d\n", itemsets[0]->count, itemsets2[0]->count);
      //printIntListList(&current);
      //itemsets.current = itemsets.current->next;
      //      printf("Incorporated locus %d\n",locus);
    }
  
   /*
  for (step = 2; step <2*loci; step *= 2)
    {
      nthr_checked=0;
      int stepper = step/2;

      int maxval = loci - stepper;
      printf("Loci=%d,Step=%d,stepper=%d,maxval=%d\n", loci,step,stepper,maxval);

      //      #pragma omp parallel for default (none) private(locus,nthr)  shared(maxval,itemsets,loci,step,stepper,nthr_checked)

    for(locus = 0; 
	locus<maxval; 
	locus +=(step))
      {
#ifdef _OPENMP
      if(nthr_checked ==0)
	{
	  nthr_checked = 1;
	  //nthr *=2;
	  //omp_set_num_threads(nthr);

	  nthr = omp_get_num_threads();
	  
	  printf( "\nWe are using %d thread(s)\n", nthr);
	}      
#endif      //lociorder[0]=locus;
      
      printf("intersecting indices %d and %d\n", locus, locus+stepper);
      //      intersectsibgroups(&current, (list*)itemsets.current->data);
      intersectsibgroups(itemsets[locus],itemsets[locus+stepper]);

      //  deleteList((list*)itemsets.current->data, free);
     
      //      printIntListList(&current);
      //      itemsets.current = itemsets.current->next;
	//printf("Incorporated locus %d, number of sets = %d\n",locus, current.count);
       char lfilename[512];
       sprintf(lfilename, "locus_current_l%d_l%d",locus,locus+stepper);
	//	prinf("locus=%d,
       writeResultsFileList(lfilename, itemsets[locus]);
      }
#pragma omp barrier
    
}*/
  double average = totaldifftime/((double)intersects);
  printf("Average intersect time for %d intersects is %.6lf seconds\n",intersects,average);
  list * final = (list*)malloc(sizeof(list));
  *final = *itemsets[0];

  

  /*  printf("\nFinished calculations.\n");
  printf("Matmul kernel wall clock time = %.2f sec\n", walltime);
  printf("Wall clock time/thread = %.2f sec\n", walltime/nthr);
  // printf("MFlops = %f\n",*/
  //  (double)(NTIMES)*(double)(N*M*2)*(double)(P)/walltime/1.0e6);

  return final;
}

/*************************
Detects and removes overlaps
 *************************/
void cleanUpOverlaps(list * source, int wipe)
{

  node * n, *n2;
  for(n = source->head; NULL!=n; n= n->next)
    {
      unsigned * x = (unsigned int*)n->data;
      for(n2 = n->next; NULL != n2; n2 = n2->next)
	{
	  int flag =0;//0- nuetral, -1 mismatch, 1 -x is dominating, 2 -y is dominating
	  unsigned int * y = (unsigned int*)n2->data;
	  int i;
	  for(i = 0; i < blocks && flag != -1; i ++)
	    {
	      unsigned int z,z2;
	      z = x[i] & y[i];
	      z2 = (x[i]| y[i]);
	      if(x[i] == y[i]) continue;
	      if(z == x[i] && z2 == y[i] ) // y is bigger
		{
		  if(flag ==0 || flag ==2)
		    flag = 2;
		  else
		    flag = -1;
		}
	      else if (z == y[i] && z2==x[i])
		{
		  if(flag == 0 || flag ==1)
		    flag = 1;
		  else
		    flag = -1;
		}
	      else
		{
		  flag = -1;
		}
	    }

	  if(flag == -1)
	    {
	      // failure
	    }
	  else if (flag == 0||flag ==1) // let's remove n2
	    {
	      node * temp = n2->previous;
	      if(wipe)
		deleteItemByNode(source, n2, free);
	      else
		deleteItemByNode(source, n2, blank);
	      n2 = temp;
	    }
	  else if (flag == 2) // let's remove n
	    {
	      node * temp = n->next;
	      if(wipe)
		deleteItemByNode(source, n,free);
	      else
		deleteItemByNode(source,n,blank);
	      n =n2=temp;
	      x = (unsigned int*)n->data;
	    }
	  
	}
    }
}

 
/***********************************************************
 *  The main 2-allele sibling reconstruction function
 *
 ***********************************************************/
struct hashtable * reconstruct(int *lociorder, int nloci )
{
  int changing = 0;
  int ecount = 0;
  int icount = 0;
  int old =0;
  int nEw=0;


  struct hashtable * sibgroupsmap = generateInitialGroups(lociorder, nloci);

  do
    {
      changing = 0;

      if(dolog)
	printf("External while: %d, no of groups here: %d\n", ecount ++,  hashtable_count(sibgroupsmap) );

      struct hashtable_itr * itr;
      itr = hashtable_iterator(sibgroupsmap);
      int groupcount = 0;
      
      do{
	   
      sibgroup * group = (sibgroup*)hashtable_iterator_key(itr); 
     
	if(dolog)
	  {
	    printf("Next Group: %d\n", groupcount++);
	  }

	if(!(*group).done)
	  {
	    int i = 0;
	    node * current = data.head;

	    for(i = 0; i < individuals && current != NULL; i ++, current = (*current).next )
	      {

		if(!(searchList((*group).assignments,i)))
		  {
		    sibgroup * temp = copySibgroup(group, nloci);

		    int result = assignIndividualToGroup(temp, (individual *) (*current).data, lociorder, nloci);
		    if(result == 1)
		      {
			assignIndividualToGroup(group,  (individual *) (*current).data, lociorder, nloci);
			addToSortedList((*group).assignments,i);
			deleteSibgroup(temp,nloci);
			if(dolog)
			  printf("successful assignment: %d\n", icount ++ );
		      }
		    else if (result == 2)
		      {
			calculateHash(temp, nloci);
			sibgroup * search;
			group->mark = 1;
			if( NULL == (search = (sibgroup*)hashtable_search(sibgroupsmap, temp)))
			    {
			      hashtable_insert(sibgroupsmap, temp, temp);
			      changing = 1;
			      if(dolog)
				printf("New Mutation assignment %d\n", nEw ++);
			    }
			else{
			  if(!searchList(search->assignments, i))
			    addToSortedList((*search).assignments, i);
			  deleteSibgroup(temp,nloci);
			  if(dolog)
			      printf("Old Mutation assignment %d\n", old ++);
			  
			}
		      }
		    else
		      {
			if(dolog)
			  printf("can't assign %d\n", i);
			deleteSibgroup(temp,nloci);
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
	      dumpGroup(group, sibgroupsmap);
		  extern void * hashtable_remove_clear(struct hashtable * h, void * k, int clear);
	      hashtable_remove_clear(sibgroupsmap, group, 0);
	      deleteSibgroup(group,nloci);
	    }
	}
      }while(hashtable_iterator_advance(itr));
      free(itr);
      printf("Number of groups at loop: %d\n", hashtable_count(sibgroupsmap));
    }while(changing);

  // now add singletons


  //  hashtable_destroy_iterator(itr);


  return sibgroupsmap;
}



/******************************************************
 * determines if the two passed sibgroups are equal
 *****************************************************/

int sibgroupEquals(void * group1_param, void * group2_param, int nloci)
{
  sibgroup* group1 = (sibgroup*) group1_param;
  sibgroup* group2 = (sibgroup*) group2_param;

  struct hashtable_itr * itr;
  int i,j;
  int*key, *v;

  if((*group1).hash != (*group2).hash)
    return 0;

  for ( i = 0; i < nloci; i ++)
    {
      if(hashtable_count(group1->intmaps[i]) != hashtable_count(group2->intmaps[i]))
	return 0;

      itr = hashtable_iterator((*group1).intmaps[i]);

      do{
	       
	key = (int*)hashtable_iterator_key(itr);
	v = (int*)hashtable_iterator_value(itr);
	int *v2;
	if(NULL == (v2 = (int*)hashtable_search((*group2).intmaps[i], key) ))
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
static unsigned int calculateHash(sibgroup * group, int nloci)
{
  struct hashtable_itr * itr;
  int i,j;
  int*key, *v;


  unsigned int hash = UINT_MAX;
  unsigned int maphash= INT_MAX;
  unsigned int phash = INT_MAX;

  

  for ( i = 0; i < nloci; i ++)
    {
      itr = hashtable_iterator((*group).intmaps[i]);
      do{
	       
	key = (int*)hashtable_iterator_key(itr);
	v = (int*)hashtable_iterator_value(itr);
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
void init(int nloci)
{
  possibilitiesMap = (struct hashtable *) create_hashtable(10, getPairInt, pairEquals ); 
  possibilities = (list *) calloc(MAX_POSSIBILITIES, sizeof(list));
  alleles = (struct hashtable **) calloc(nloci, sizeof(struct hashtable *));
  int i;
  for(i = 0; i < nloci; i ++)
    {
      alleles[i] = (struct hashtable *) create_hashtable(10, getIntHash, equals);
    }
  
  costs = (double*)calloc(6, sizeof(double));
  costs [NULL_ALLELE_ERROR]= 0.1;
  costs[HOMO_MISTYPE_ERROR]= 0.3;
  costs[HETERO_MISTYPE_ERROR] =0.15;
  costs[POSSIBILITY_MISFIT]= 0.4;
  maxedit = 0.5;
  //lociorder = NULL;
  populateMoveMap( );
}

/***************************************************
determines if the given two pairs have same values
 *************************************************/
int pairEquals(void * p1_param, void * p2_param, int dummy_unused)
{
  Pair *p1 = (Pair*) p1_param;
  Pair *p2 = (Pair*) p2_param;
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
// APR - i <3 signed to unsigned conversions! 
unsigned int getPairInt(void * pair_param)
{
  Pair* pair = (Pair*) pair_param;
  int x = 0;
  x = (*pair).x;
  x *= 100;
  x+= (*pair).y;
  return (unsigned int)x;
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
  node * newnode = (node*)malloc(sizeof(node));
  (*newnode).next = NULL; //(*addTo).head;
  (*newnode).previous = NULL; //(*addTo).head;
  
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
  node * newnode = (node*)malloc(sizeof(node));
  (*newnode).next = NULL; //(*addTo).head;
  
  node * current;
  current = addTo->head;
  while(current != NULL && ((int) current->data)<x && current->next != NULL) current = current->next;
  // APR - very fishy again, comparing a void* to a straight int, casting void* to int though this may be wrong...
  if(current != NULL && (int)current->data == x)
    return;
  //  newnode->next = newnode->previous = current;
  
  if(( (*addTo).head == NULL || current->previous == NULL ) && ( current == NULL ||((int)current->data) > x))
  {
    addTo->head = newnode;
    if(current != NULL)
      {
	current->previous = newnode;
      }
    newnode->previous = NULL;
    newnode->next = current;
  }
  // APR - very fishy again, comparing a void* to a straight int, casting void* to int though this may be wrong...
  else if((int)current->data >= x)
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
  char * buffer = (char*)malloc(sizeof(char) * 512);
  int i = 0;
  int count = 0;
  char *allele1, * allele2, * read;


  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data %s\n", filename);
      exit(-1);
    }
  else{
    if(dolog)
      {

	printf("Successfully opened file\n");
      }
  }

  /*char * progress;*/
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

	Pair * pair = (Pair*)malloc(sizeof(Pair));
	// APR - function is no longer reentrant? Is this being called from different threads?
	allele1 = strtok/*_r*/(buffer, delims/*, &progress*/);
	allele2 = strtok/*_r*/(NULL, delims/*, &progress*/);
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
	  int * possibs = (int*)hashtable_search(possibilitiesMap, pair);
	  if(possibs == NULL)
	    {
	      possibs = (int*)calloc(MAX_POSSIBILITIES, sizeof(int));
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
  char* buffer = (char*)malloc(sizeof(char) * 512);
  char * name,*allele1, *allele2/*, *progress*/;

  int index = 0;
  char* read = 0;
  data.head= NULL;
  data.count = 0;
  int counter = 0;
  char delims[] = "\t, /\n\r";

  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data %s\n", filename);
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

      
      if(strlen(buffer) <= 3)
	return;
	  // APR - this is no longer re-entrant?  Are we calling this from multiple threads?
      name = strtok/*_r*/(buffer, delims/*, &progress*/);
      dataentry * j = (dataentry*)malloc(sizeof(dataentry));
      strcpy(j->name, (const char*) name);
      
      (*j).loci = (Pair*)calloc(nloci, sizeof(Pair));
      index = 0;
      
      /* Extract remaining 					 * strings 		*/
      Pair * pair = (*j).loci;
      j->index = counter ++;
      while ( (allele1 = strtok/*_r*/(NULL, delims/*, &progress*/)) != NULL && index < nloci)
	{
	  
	  
	  allele2=strtok/*_r*/(NULL,delims/*, &progress*/);
	  if(allele2 == NULL)
	    {
	      fprintf(errors, "Urgghhh.... cant do this, insufficient data\n");
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

	  int * k1 = (int*)malloc(sizeof(int));
	  *k1 = pair->x;
	  int * k2 = (int*)malloc(sizeof(int));
          *k2 = pair->y;
	  
	  int * val1 = (int*)hashtable_search(alleles[index], k1);
	  if(val1 == NULL)
	    {
	      val1 = (int*)malloc(sizeof(int));
	      *val1 = 0;
	      hashtable_insert(alleles[index], k1, val1);
	    }
	  else
	    free(k1);
	  (*val1)++;
	  // APR - pretty sure this is a bug, should be k2 not k1
	  int * val2 = (int*)hashtable_search(alleles[index], k2);
          if(val2 == NULL)
            {
              val2 = (int*)malloc(sizeof(int));
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
  char* buffer = (char*)malloc(sizeof(char) * 512);
  char * name,*allele1, *allele2/*, *progress*/;

  int index = 0;
  int assignedindex = 0;
  char* read = 0;
  data.head= NULL;
  data.count = 0;
  int counter = 0;
  char delims[] = "\t, /\n\r";

  if((infile = fopen(filename, "r")) == NULL)
    {
      printf( "Error can't open file for reading data %s\n", filename);
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
	  // APR - this function is no longer reentrant? Is this called from multiple threads?
      name = strtok/*_r*/(buffer, delims/*, &progress*/);
      dataentry * j = (dataentry*)malloc(sizeof(dataentry));
      strcpy(j->name, (const char*) name);
      
      (*j).loci = (Pair*)calloc(nloci, sizeof(Pair));
      index = 0;
      assignedindex=0;
      
      /* Extract remaining 					 * strings 		*/
      Pair * pair = (*j).loci;
      j->index = counter ++;
	  // APR - this function is no longer reentrant? Is this called from multiple threads?
      while ( (allele1 = strtok/*_r*/(NULL, delims/*, &progress*/)) != NULL && assignedindex < nloci && index < nloci+1)
	{


	  
	  allele2=strtok/*_r*/(NULL,delims/*, &progress*/);
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

	  int * k1 = (int *)malloc(sizeof(int));
	  *k1 = pair->x;
	  int * k2 = (int *)malloc(sizeof(int));
          *k2 = pair->y;
	  
	  int * val1 = (int *)hashtable_search(alleles[assignedindex], k1);
	  if(val1 == NULL)
	    {
	      val1 = (int *)malloc(sizeof(int));
	      *val1 = 0;
	      hashtable_insert(alleles[assignedindex], k1, val1);
	    }
	  else
	    free(k1);
	  (*val1)++;
	  int * val2 = (int *)hashtable_search(alleles[assignedindex], k1);
          if(val2 == NULL)
            {
              val2 = (int *)malloc(sizeof(int));
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
int equals(void* x, void* y, int dummy_unused)
{
	return *((int*)x) == *((int*)y);
}

/*********************
returns the value of the int as hash for hash table

params: x the value which is simply returned
**********************/
 unsigned int getIntHash(void * x)
{
	return 5000-*((int*)x);
}

/**************************************************************
  returns possibilities table entries with the given pair
 *************************************************************/
int * lookupPossibilities(Pair * p)
{
 
  int * vals = (int *)hashtable_search(possibilitiesMap, p);
  if(vals == NULL)
    vals = EMPTY_POSSIBILITIES;
  return vals;
}


/****************************************************************
Initializes a sibgroup

params:
group: pointer to the sibgroup which will be initialized
***************************************************************/
void initializeSibgroup(sibgroup * group, int nloci)
{

	int i,j,k;
	(*group).possibilitysets = (int **)calloc(nloci, sizeof(int *));
	(*group).allelecounts = (int *)calloc(nloci, sizeof(int));	
	(*group).intmaps = (struct hashtable**)calloc(nloci, sizeof( struct hashtable *));
	group->edit = 0;
	group->mark = 0;
	group->mutate = 0;
	for(i = 0; i < nloci; i ++)
	{
	  (*group).allelecounts[i] = 1;
	  ((*group).intmaps)[i] = newAlleleMap(i);
	  (*group).possibilitysets[i] = (int *)calloc(MAX_POSSIBILITIES, sizeof(int));
	  for(j = 0; j < MAX_POSSIBILITIES; j++)
	    {
	      (*group).possibilitysets[i][j]= 1;
	    }
	}

	(*group).assignments = (list*)malloc(sizeof(list));//calloc(individuals, sizeof(int ));
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
	int * result = (int *)calloc(MAX_POSSIBILITIES, sizeof(int));
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
sibgroup* copySibgroup(sibgroup * source, int nloci)
{
  sibgroup* group = (sibgroup*) malloc(sizeof(sibgroup));
  int i,j,k,*key,*v;
  group->mark = 0;
  struct hashtable_itr * itr;
  (*group).possibilitysets = (int **)calloc(nloci, sizeof(int *));
  (*group).allelecounts = (int *)calloc(nloci, sizeof(int));
  (*group).founder = source->founder;
  (*group).intmaps = (struct hashtable**)calloc(nloci, sizeof( struct hashtable *));
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
            key = (int *)hashtable_iterator_key(itr);
            v = (int *)hashtable_iterator_value(itr);
	    int * k2 = (int *)malloc(sizeof(int));
	    *k2 = *key;
	    int * v2 = (int *)malloc(sizeof(int));
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
   
      
      (*group).possibilitysets[i] = (int *)calloc( MAX_POSSIBILITIES, sizeof(int) );
      for(j = 0; j < MAX_POSSIBILITIES; j++)
	{
	  (*group).possibilitysets[i][j]= (*source).possibilitysets[i][j];
	}
    }
  
  (*group).assignments = (list*)malloc(sizeof(list));//calloc(individuals, sizeof(int));
  copyList(group->assignments, source->assignments);
  

  return group;
}

 void blank(void *k){}

/*************************************************************
 Frees memory used by a sibgroup
************************************************************/
void deleteSibgroupShallow(sibgroup * group, int shallow,int nloci)
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

void deleteSibgroup(sibgroup * group, int nloci)
{
  deleteSibgroupShallow(group, 0,nloci);
}

void datacleaner(void * p)
{
  dataentry * e = (dataentry*) p;
  free(e->loci);
  free(e);
}

/**********************
 deletes a given list with and using the given 'cleaner' function to free up data

*******************************************/
void deleteList(list * target, void cleaner( void* ) )
{
  node * current = (*target).head;
  while(current != NULL)
    {
      node * next = (*current).next;
      cleaner((*current).data);
      free(current);
	  current = next;
    }
  target->head=NULL;
  target->count = 0;
}

/*******************************************
 newAlleleMap
 builder function for allelemaps, takes care of wild card mapping
**************************************************/
struct hashtable * newAlleleMap(int loc )
{
  struct hashtable * map = (struct hashtable *) create_hashtable(6, getIntHash, equals );  
  int * key = (int*)malloc(sizeof(int));
  int * value = (int*)malloc(sizeof(int));
  *key = -1;
  *value = 9;
  hashtable_insert(map, key, value);

  if(mothersmode){
    if(mother->loci[loc].x == mother->loci[loc].y)
      {
	int * key1 = (int*)malloc(sizeof(int));
	int * val1 = (int*)malloc(sizeof(int));
	*key = mother->loci[loc].x;
	*val1 = 1;
	hashtable_insert(map, key1, val1);
      }
    else{
      int * key1 = (int*)malloc(sizeof(int));
      int * val1 = (int*)malloc(sizeof(int));
      int * key2 = (int*)malloc(sizeof(int));
      int * val2 = (int*)malloc(sizeof(int));
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
int assignIndividualToGroup(sibgroup * group, individual * indi, int * lociorder, int nloci)
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
      allelemappings = (*group).intmaps[loc];
      possibilities = (*group).possibilitysets[loc];
      Pair mappedpair;
      
      if(NULL == (mappedvalue = (int*)hashtable_search(allelemappings, & (*indi).loci[locus].x)))
	{
	  modified = 1;
	  
	  if((*group).allelecounts[loc] < MAX_ALLELES_PER_LOCUS)
	    {
	      mappedpair.x = adjustAlleles(group, loc, indi->loci[locus].x);	      
	    }
	  else{
	    failed = 1;
	    break;
	  }
	}else
	{
	  mappedpair.x = *mappedvalue;
	}

      if(NULL == (mappedvalue = (int*)hashtable_search(allelemappings, & (*indi).loci[locus].y)))
	{
	  modified = 1;
	  
	  if((*group).allelecounts[loc] < MAX_ALLELES_PER_LOCUS)
	    {
	      mappedpair.y = adjustAlleles(group, loc, indi->loci[locus].y);
	    }
	  else
	    {
	      failed = 1;
	      break;
	    }
	}
      else
	{
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
	  int * intersection = intersectPossibilities((*group).possibilitysets[loc], lookeduppossibilities);
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
	  else if(!equalsPossibilitySets(intersection, group->possibilitysets[loc]))
	    {
	      modified = 1;
	      free((*group).possibilitysets[loc]);
	  
	      (*group).possibilitysets[loc] = intersection;
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
void writeResultsFile(const char * filename, struct hashtable* sibgroupsmap)
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
  struct hashtable_itr * itr;

  itr = hashtable_iterator(sibgroupsmap);
  int groupcounter = dumpgroupcounter;
  do{
    sibgroup * group =(sibgroup*)hashtable_iterator_key(itr);
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
      logGroup(logfile, group, nloci);
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


/******************************************************
   Writes results to file in a tab separated format
 ****************************************************/
void writeResultsFileList(const char * filename, list * setslist)
{
  FILE * file;// = dumpfile;
  FILE * logfile;

  if( NULL == (file = fopen( filename, "w")))
    {
      printf("Unable to write results\n");
      exit(-1);
    }

  if(dolog)
    {
      printf("Opened results file for writing");
      logfile= fopen("siblings.log", "w");
    }
  //  struct hashtable_iterator * itr;

  //  itr = hashtable_iterator(sibgroupsmap);
  int groupcounter = dumpgroupcounter;
  for(setslist->current = setslist->head; setslist->current != NULL; setslist->current = setslist->current->next)
  {
    unsigned int * group = (unsigned int*)setslist->current->data;//hashtable_iterator_key(itr);
    int i = 0;
    //    group->mark = groupcounter;
    fprintf(file, "\"Set(%d):\"", groupcounter ++);

    unsigned int * p = group;
    i = 0;
    while(i < individuals)
      {
	unsigned int ndata =(unsigned int) *p;
	int index;
	unsigned int bits = 1;
	int shift = (i+(sizeof(unsigned int)*8)>individuals)?(individuals- i):sizeof(unsigned int)*8;
	//	printf("shift: %d\n", shift);
	shift --;
	bits <<= shift;
	//	printf("bits:%X\n",bits);
	for(index = 0; index < sizeof(unsigned int)*8 && i < individuals; i ++,index++)
	  {
	    int selected = ((bits & ndata)>0 ? 1 : 0);
	    fprintf(file, "\t%d", selected );
	    //	    printf("Ind i:%d selected:%d\n",i,selected);
	    bits >>= 1;
	  }

	p ++;
      }
	// APR - WTF - doesn't compile.
    //if(dolog)
    //  logGroup(logfile, group,nloci);
    fprintf(file, "\n");
  }// end groups for loop

  /// now add singletons

  /*
  int i,j;
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
  */
}


/***********************************************
 * performs initialization needed for min fathers.
 ***********************************************/
void initMother()
{
  mothersmode = 1;
  int i=0;
  motherpossibsmap = (int**)calloc(nloci, sizeof(int*));
  for(i = 0; i < nloci; i ++)
    {
      motherpossibsmap[i] = (int*)calloc(MAX_POSSIBILITIES, sizeof(int));
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
  struct hashtable_itr * itr;
  itr = hashtable_iterator(sets);
  for(i = 1;  ; i++)
    {
      sibgroup * group = (sibgroup*)hashtable_iterator_key(itr);
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

void writeGamsFileFromSetsList(const char * filename, list * setslist)
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
  int m = setslist->count;
  if(setslist->count == 0)
    {
      printf ("ERROR! No sets were generated\n");
      exit(-1);
    }
  int i,j;
  fprintf(fid, "$ONEMPTY \n \n");
  fprintf(fid, "SET i   sibling   /s1*s%d/; \n",n);
  fprintf(fid, "SET j   sibset    /t1*t%d/; \n \n",m);
  fprintf(fid, "TABLE A(j,i)    sets covered by node i \n");
  fprintf(fid, "     ");
  for(i=1; i <= n; i++)
	 fprintf(fid, "\t s%d",i);
  fprintf(fid, " \n");
  //  struct hashtable_iterator * itr;
  //itr = hashtable_iterator(sets);
  node * itemset;
  for(i = 1, itemset = setslist->head; itemset!= NULL ; i++, itemset=itemset->next)
    {
      //      sibgroup * group = hashtable_iterator_key(itr);
      unsigned int * iset = (unsigned int*)itemset->data;
      fprintf(fid, "t%d",i);

      unsigned int * p = iset;
      int j = 0;
      while(j < individuals)
      {
	unsigned int ndata =(unsigned int)* p;
	int index;
	//unsigned int bits = 1;
	int shift = (j+(sizeof(unsigned int)*8)>individuals)?(individuals- j):sizeof(unsigned int)*8;
	shift --;

	//	bits <<= shift;
	for(index = 0; index < sizeof(unsigned int)*8 && j < individuals; j ++,index ++)
	  {	    
	    fprintf(fid, "\t %d", (ndata>>shift)&1 );
	    //bits >>=1;
	    shift --;
	  }
	//	if(p && ((int) p->data) == i)
	p++;
      }

      fprintf(fid, " \n");
    }
  i++;
  //  free(itr);
  for(; i <=m ; i ++)
    {
      //      fprintf(fid, "\"Set(%d):\"", groupcounter ++); 
      //      sibgroup * group = hashtable_iterator_key(itr);
      //	 fprintf(fid, "t%d",i);
      fprintf(fid, "t%d",i);
      for ( j = 0; j < n; j ++)
	{
	     int assignment = 0;
	     if(i-  setslist->count -1 == j)
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
    fprintf(fid, "VARIABLE TF; \n\n");
    //    fprintf(fid, " \n");
    fprintf(fid, "setcov(i).. \n");
    fprintf(fid, "sum(j, A(j,i)*xset(j)) =g= 1; \n\n");
    //    fprintf(fid, " \n");
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
      ll->head = next;
      next->previous=NULL;
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
  ll->count --;
}


void addToVector(vector * vec, unsigned int ** intlist)
{
  if(vec->count == vec->psize)
    {
      vec->psize *= 2;
      unsigned int *** p = (unsigned int***)calloc(vec->psize, sizeof(unsigned int**));
      int i;
      for ( i = 0; i < vec->count; i ++)
	{
	  p[i]= vec->data[i];
	}
      free(vec->data);
      vec->data = p;
    }
  vec->data[vec->count]=intlist;
  vec->count ++;
}

vector * newVector(int size)
{
  vector * v = (vector*)malloc(sizeof(vector));
  v->count = 0;
  v->data = (unsigned int***)calloc(size, sizeof(int **));
  return v;
}


/***********************

*************************/
void cleanUp(struct hashtable * sibgroupsmap)
{
	// need to write this to clean up the data etc
  struct hashtable_itr * itr = hashtable_iterator(sibgroupsmap);
  do{
    sibgroup * group = (sibgroup *)hashtable_iterator_key(itr);
    deleteSibgroupShallow(group, 1,sibgroupsmap->nloci);
  }while(hashtable_iterator_advance(itr));
  free(itr);
  hashtable_destroy(sibgroupsmap, 0);
  sibgroupsmap = NULL;
  
}

struct hashtable * intersectsibgroupsIntList(list * currentList, list * sibgroupslist)
{
  struct hashtable * intersection;
  intersection = (struct hashtable *) create_hashtable(sibgroupslist->count, getIntListHash, equalsIntList); 

  list newlist;
  newlist.head=NULL;
  newlist.current = NULL;
  newlist.count = 0;

  for(currentList->current = currentList->head; currentList->current != NULL;
      currentList->current = currentList->current->next)
    {
      list * xlist = (list*) currentList->current->data;
  
      for(sibgroupslist->current = sibgroupslist->head; sibgroupslist->current != NULL;
	  sibgroupslist->current = sibgroupslist->current->next)
	{
	  
	  list * ylist = (list*)sibgroupslist->current->data;
	  
	  
	  list intersectedList;
	  intersectedList.head = NULL;
	  intersectedList.current = NULL;
	  intersectedList.count = 0;
	  {
	    // comparison
	    xlist->current = xlist->head;
	    ylist->current = ylist->head;

	    while(xlist->current != NULL && ylist->current != NULL)
	      {
		int xval = (int) xlist->current->data;
		int yval = (int) ylist->current->data;

		if(xval == yval)
		  {
		    addToSortedList(&intersectedList, xval);
		    xlist->current = xlist->current->next;
		    ylist->current = ylist->current->next;
		  }
		else if(xval < yval)
		  {
		    xlist->current = xlist->current->next;
		  }
		else if(xval > yval)
		  {
		    ylist->current = ylist->current->next;
		  }
	      }

	  }

	  if(intersectedList.count>0 && NULL == hashtable_search(intersection, &intersectedList))
	    {
	      list * copy = (list*)malloc(sizeof(list));
	      *copy = intersectedList;
	      hashtable_insert(intersection, copy, copy);
	      addToList(&newlist, copy);
	    }
	}// end sibgroups for loop
      
    }// end current list for loop

  list intersectedList;
  intersectedList.head = NULL;
  intersectedList.current = NULL;
  intersectedList.count = 0;
  deleteList(currentList, blank);
  /*struct hashtable_iterator * itr;
  
   itr = hashtable_iterator(intersection);
  do{
    list * grp= (list*)hashtable_iterator_key(itr);
    addToList(currentList, grp);
  }while(hashtable_iterator_advance(itr));
  free(itr);*/
  //  hashtable_destroy(intersection, 0);
  *currentList = newlist;
	
  return intersection;
}
