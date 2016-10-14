/********************************
 * main.c
 * Author: Saad Sheikh 
 *
 * contains the main function and all the global variables
 * 
 * Created: July 23, 2007
 ********************************/
#include <stdlib.h>
//#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "structures.h"
#include "main.h"
#include "hashtable.h"
#include "hashtable_itr.h"
#include "utilities.h"

list data;
struct hashtable * possibilitiesMap;
int nloci;
int * lociorder;
int dolog;
int mothersmode = 0;
individual * mother;
int individuals = 0;
int * allassignments;
struct hashtable * sibgroupsmap;

int ** motherpossibsmap;
struct hashtable ** alleles;
list * possibilities;
const int MAX_POSSIBILITIES=15;
const int ABSENT_ALLELE = -1;
const int MAX_ALLELES_PER_LOCUS = 5;

double * costs;
double maxedit;

FILE * logfile;

// displays the usage guide
void showusage( )
{
  printf("usage: sets test/father nloci juvfilename setfilename [on/off] [siblings-completefile]\n");
  printf("or: sets consensus nloci juvfilename solfile solfile2 ...\n");
  printf("Note: father mode requires an individual names 'Mother'\n");
}

void blanko(void * x){}

int map(int x)
{
  if(x == 9)
    return ABSENT_ALLELE;
  else return x;
}

void testList()
{
  list mylist;
  mylist.head = 0x0;
  mylist.count = 0;
  mylist.current = 0x0;
  int i= 0;
  
  addToSortedList(&mylist, 1804289383);
  addToSortedList(&mylist, 846930886);
  searchList( &mylist, 846930886);
  list temp;
  copyList( &temp, &mylist);
  searchList(&temp, 846930886);
  for( i = 0; i <= 2000; i ++)
    {
      int x = rand();
      addToSortedList(&mylist, x);
      list temp;
      copyList(&temp,& mylist);
      if(!searchList( &mylist, x) || !searchList(&temp, x))
	{
	  printf ("\nERROR %d \n", x);
	  i = 2001;
	}
      else 
	{
	  printf("%d\t", x);
	}
      deleteList(& temp, &blanko);
    }
}

void testOverlaps()
{
  blocks = 2;
  unsigned x =0xFFFF;

  unsigned y = 0xABBB;
  unsigned z = 0xA000;

  unsigned int* l = calloc(2, sizeof(unsigned int));
  l[0]=y;
  l[1]=z;
  /*  list * l = newList();
  addToList(l, &y);
  addToList(l, &z);
  */

  unsigned int* l2=calloc(2, sizeof(unsigned int));
  l2[0]=z;
  l2[1]=y;

  /*  list * l2 = newList();
  addToList(l2, &z);
  addToList(l2, &y);*/

  unsigned int* l3=calloc(2, sizeof(unsigned int));
  l3[0]=z;
  l3[1]=z;

  unsigned int* l4=calloc(2, sizeof(unsigned int));
  l4[0]=x;
  l4[1]=y;

  /*  list * l3 = newList();
  addToList(l3, &z);
  addToList(l3, &z);
  */

  list * ll = newList();
  addToList(ll, l3);
  //  addToList(ll, l4);
  addToList(ll, l);
  addToList(ll, l2);

  cleanUpOverlaps(ll,0);

  printf("%d element(s) in list\n", ll->count);

  node * n = ll->head;
  for(; n != NULL; n = n->next)
    {
      unsigned int * d = (unsigned int*) n->data;
      int i;
      for (i = 0; i < blocks; i ++)
	{
	  printf("%x ",d[i]);
	}
      printf("\n");
    }
  

}

	      

/*
void test5()
{
  int i;
  nloci = 1;
  init();
  sibgroupsmap = NULL;
  readPossibilities("siblings-complete.csv");
  
  for(i = 1; i < MAX_POSSIBILITIES; i ++)
    {

      
      list * possibilityset = & possibilities[i];
      if(sibgroupsmap != NULL)
	{
	   hashtable_destroy(sibgroupsmap, 0);
	   sibgroupsmap = NULL;

	}
	  individuals = possibilityset->count;

     
      int j = 0;
      int found = 0;
      node * current = possibilityset->head;
      for(j = 0; j < individuals; j ++, current = current->next)
	{
	  dataentry * je = malloc(sizeof(dataentry));
	  //strcpy(je->name, "whatever");
	  je->name[1]='\0';
	  (*je).loci =malloc( sizeof(Pair));

	  je->loci->x =map( ((Pair*)current->data)->x);
	  je->loci->y =map( ((Pair*)current->data)->y);

	  addToList(&data, je);
	}

      reconstruct();

      struct hashtable_iterator * itr;
      itr = hashtable_iterator(sibgroupsmap);
      do{
	sibgroup * group = hashtable_iterator_key(itr);
	int nopes = 0;
	printf("Group:");
	for(j = 0; j < individuals ; j ++)
	  {
	
	    if(group->assignments[j] == 0)
	      nopes = 1;
	    printf(" %d",group->assignments[j]);
	  }
	printf("\n");
	if(nopes == 0)
	  {
	    found = 1;
	    break;
	  }
      }while(hashtable_iterator_advance(itr) && !found);

      if(found == 0)
	{
	  printf("ERROR WHILE RECONSTRUCTING SIBGROUP for possib %d\n", i);
	}
      else printf("Possib: %d successful\n", i);
      
    }
}
*/
void test4()
{
  init();
  dolog = 1;
  nloci = 2;
  readPossibilities("siblings-complete.csv");
  
  Pair p;
  p.x = 9;
  p.y = 9;
  int * possibs = lookupPossibilities(& p);
  int i = 0;
  for(i = 0; i < MAX_POSSIBILITIES; i ++)
    {
      printf("possibs x:%d y:%d i:%d=>%d\n", p.x, p.y,i, possibs[i]);
    }
}
/*
void test3( )
{

  sibgroupsmap = (struct hashtable *) create_hashtable(100, sibgroupHash, sibgroupEquals); 
  int i =0, j =0;
  int max = 100;
  nloci = 5;
  init();
  for(i = 0; i < max; i ++)
    {
      sibgroup * group= malloc(sizeof(sibgroup));
      initializeSibgroup(group);
      for(j =0; j < nloci; j ++)
	{
	  int k = 0;

	  printf("locus %d\n", j);
	  //	  for(k=1; k < MAX_ALLELES_PER_LOCUS; k ++)
	    {
	      int * key1 = malloc(sizeof(int));
	      int * value1 =malloc(sizeof(int));
	      *key1 = rand() % 400;
	      *value1 = k++;
	      hashtable_insert( group->intmaps[j] , key1, value1);
	      printf("%d -> %d\t", *key1, *value1);

	      int * key2 = malloc(sizeof(int));
	      int * value2 =malloc(sizeof(int));
	      *key2 = rand() % 400;
	      *value2 = k++;
	      hashtable_insert( group->intmaps[j] , key2, value2);
	      printf("%d -> %d\t", *key2, *value2);

	      int * key3 = malloc(sizeof(int));
	      int * value3 =malloc(sizeof(int));
	      *key3 = rand() % 400;
	      *value3 = k++;
	      hashtable_insert( group->intmaps[j] , key3, value3);
	      printf("%d -> %d\t", *key3, *value3);

	      int * key4 = malloc(sizeof(int));
	      int * value4 =malloc(sizeof(int));
	      *key4 = rand() % 400;
	      *value4 = k++;
	      hashtable_insert( group->intmaps[j] , key4, value4);
	      printf("%d -> %d\n", *key4, *value4);
	    }
	  
	  int a1 = rand() % 10;
	  int a2 = rand() % 10;
	  int a3 = rand() % 10;
	  int a4 = rand() % 10;
      
	  group->possibilitysets[j][a1] =1;      
	  group->possibilitysets[j][a2] =1;      
	  group->possibilitysets[j][a3] =1;
	  group->possibilitysets[j][a4] =1;
	  
	  group->mutate = i;
	  
	  
	  

	}

      
      hashtable_insert(sibgroupsmap, group, group);
      sibgroup * fish ;
      if((fish = hashtable_search(sibgroupsmap, group)) == NULL || fish->mutate != i)
	{
	  hashtable_search(sibgroupsmap, group);
	  printf("ERRORRRRR\n");
	}
    }
}

void test( )
{
  long i;
  while (1)
    {
      sibgroup * test = malloc(sizeof(sibgroup));
      initializeSibgroup(test);
      sibgroup * copy = copySibgroup(test);
      deleteSibgroup(test);
      deleteSibgroup(copy);
      
    }
}


void test2()
{

  //  test();
  init();
  dolog = 1;
  nloci = 2;
  readPossibilities("siblings-complete.csv");
  readData("mixednuts.csv");

  reconstruct();
  writeResultsFile("blah");

}
*/
void testparallel()
{

  individuals = 50;
  data.count = 50;
  int i;
  list listy;
  listy.head = NULL;
  listy.count = 0;
  listy.current = NULL;
  for(i = 0; i < individuals; i ++)
    {
      addToSortedList(&listy, i);
    }

  blocks = 2;
  unsigned int * coded = encodeList(&listy);
  printf("Coded val for first block = %X, second block = %X\nMax Value for unsigned int %X\n", coded[0],coded[1],UINT_MAX);

  list listy2;
  listy2.head = NULL;
  listy2.count = 0;
  listy2.current = NULL;
  addToSortedList(&listy2, 5);
  addToSortedList(&listy2, 4);
  addToSortedList(&listy2, 7);
  addToSortedList(&listy2, 45);


  unsigned int * coded2 = encodeList(&listy2);
  printf("Coded val for first block= %X\tsecond block %X\n", coded2[0], coded2[1]);

  list listy3;
  listy3.head=NULL;
  listy3.current = NULL;
  listy3.count = 0;
  addToList(&listy3, coded);

  list listy4;
  listy4.head=NULL;
  listy4.current = NULL;
  listy4.count = 0;
  addToList(&listy4, coded2);



  list listy6;
  listy6.head = NULL;
  listy6.count = 0;
  listy6.current = NULL;
  addToSortedList(&listy6, 5);
  addToSortedList(&listy6, 14);
  addToSortedList(&listy6, 7);
  addToSortedList(&listy6, 41);


  list listy7;
  listy7.head = NULL;
  listy7.count = 0;
  listy7.current = NULL;
  addToSortedList(&listy7, 5);
  addToSortedList(&listy7, 14);
  addToSortedList(&listy7, 7);
  addToSortedList(&listy7, 41);

  
  unsigned int * coded3 = encodeList(&listy6);
  addToList(&listy4, coded3);
  writeResultsFileList("listy4", &listy4);  
  intersectsibgroups(&listy4, &listy3);

  unsigned int * x = listy4.head->data;
  printf("Coded val for first block= %X\tsecond block %X\nequals %d", x[0],x[1],equalsArray(x,coded2) );
  writeResultsFileList("listy3", &listy3);
  writeResultsFileList("listy4-2", &listy4);
  


  return;

  //void intersectsibgroups(list * currentList, list * sibgroupslist)
  /* list  x;
  x.head = NULL;
  x.count = 0;
  x.current = NULL;
  
  list y;
  y.head = NULL;
  y.count = 0;
  y.current = NULL;

  list z;
  z.head = NULL;
  z.count = 0;
  z.current = NULL;


  list alpha;
  alpha.head = NULL;
  alpha.count = 0;
  alpha.current = NULL;
  
  list beta;
  beta.head = NULL;
  beta.count = 0;
  beta.current= NULL;
  
  individuals = 6;
  data.count = 6;

  addToSortedList(&x, 1);
  addToSortedList(&x, 2);
  addToSortedList(&x, 4);

  addToSortedList(&y, 1);
  addToSortedList(&y, 3);
  addToSortedList(&y, 5);

  addToSortedList(&z, 1);
  addToSortedList(&z, 4);
  addToSortedList(&z, 5);


 unsigned int * x1=  encodeList(&x);
 unsigned int * y1= encodeList(&y);
 unsigned int * z1= encodeList( &z);

 printf("x1=%d y1=%d z1=%d\n", *x1, *y1, *z1);

  addToList(&alpha, x1);

  addToList(&alpha, z1);
  addToList(&beta, y1);

  intersectsibgroups(&beta, &alpha);

  

   if( NULL == (dumpfile = fopen( "dump", "w")))
        {
          printf("Unable to write results\n");
          exit(-1);
        }
   writeResultsFileList("dump", &beta);
*/  

  nloci =2;
  init();
  readPossibilities("siblings-complete.csv");
  readData("juv_test1");
   list * finallist = lociConstruct("juv_test1", 2);
  // writeResultsFileList("dump", finallist);
   if( NULL == (dumpfile = fopen( "dump", "w")))
        {
          printf("Unable to write results\n");
          exit(-1);
        }
   writeResultsFileList("dump", finallist);
  
}

struct rusage usage;

void dumpusage(const char * filename)
{
  FILE * dfile = fopen(filename, "w");
  fprintf(dfile, "User\t%d\t%d\n",usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
  fprintf(dfile, "System\t%d\t%d\n",usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
  fprintf(dfile, "MaxResSize\t%d\n",usage.ru_maxrss);
  fprintf(dfile, "SharedMemSize\t%d\n",usage.ru_ixrss);
  fprintf(dfile, "InpOps\t%d\n", usage.ru_inblock);
  fprintf(dfile, "OutpOps\t%d\n", usage.ru_oublock);
  fprintf(dfile, "Swaps\t%d\n",usage.ru_nswap);
  fclose(dfile);
}

// main function
int main(int argc, char ** argv)
{

  int i = 0;
  if(dolog)
  for(i = 0; i < argc; i ++)
    {
      printf("Argument %d is %s\n", i, argv[i]);
    }

  if(argc<3)
    {
      testOverlaps();
      return 0;
    }
  else
    {
      nloci = atoi( argv[1]);
     
      init(nloci);
      readData(argv[2]);
      readPossibilities("siblings-complete.csv");
      if( NULL == (dumpfile = fopen( argv[3], "w")))
        {
          printf("Unable to write results\n");
          exit(-1);
        }

      list * finallist = lociConstruct(argv[2], nloci);
      writeResultsFileList(argv[3], finallist);
      char gamsname[512];
      strcpy(gamsname, argv[3]);
      strcat(gamsname, ".gms");
      writeGamsFileFromSetsList(gamsname, finallist);
      
      getrusage(RUSAGE_SELF, &usage);
      if(argc>4)
	dumpusage(argv[4]);
      else
	dumpusage("usage");
      return 0;
    }
}
