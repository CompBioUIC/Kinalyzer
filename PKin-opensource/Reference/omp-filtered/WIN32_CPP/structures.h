/*******************************************************************
 * structures.h
 * author: saad sheikh
 * Date Created: July 23, 2007
 *
 * Contains definition of all the structures
 ******************************************************************/
#include "hashtable.h"

#ifndef STRUCTURES_H
#define STRUCTURES_H

struct struct_pair
{
  int x;
  int y;
};

typedef struct struct_pair Pair;

struct struct_data
{
  int index;
  char name[512];
  Pair * loci;
  
};


typedef struct struct_data dataentry;
typedef struct struct_data individual;

struct struct_list_node
{
  struct struct_list_node * next;
  struct struct_list_node * previous;
  void * data;

};

typedef struct struct_list_node node;

struct struct_list
{
  node * head;
  node * current;
  int count ;
};

typedef struct struct_list list;

struct struct_itemset{

  struct hashtable ** intmaps;
  int ** possibilitysets;
  list * assignments;
  int * allelecounts;
  unsigned int hash;
  int done;
  int mark;
  Pair founder;
  int mutate;
  int mutationstamp;
  int mutation;
  double edit;
};

typedef struct struct_itemset sibgroup;

struct struct_vector{
  int count;
  int psize;
  unsigned int *** data;
};

typedef struct struct_vector vector;

#endif
