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

struct rusage usage;

void dumpusage(const char * filename)
{
  FILE * dfile = fopen(filename, "w");
  fprintf(dfile, "User\t%d\t%d\n",usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
  fprintf(dfile, "System\t%d\t%d\n",usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
  fclose(dfile);
}


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
      deleteList(& temp, blanko);
    }
}

void testConsensus()
{
  nloci=7;
  init();
  readCosts(NULL);

  readPossibilities("siblings-complete.csv");
  char * f1 = "set_m_1.soln";
  char * f2 = "set_m_2.soln";
  char * f3 = "set_m_3.soln";
  char * f4 = "set_m_4.soln";
  char * f5 = "set_m_5.soln";
  char * f6 = "set_m_6.soln";
  char * f7 = "set_m_7.soln";
  char ** p = calloc(7, sizeof(char*));
  p[0]= f1;
  p[1] = f2;
  p[2] = f3;
  p[3] = f4;
  p[4] = f5;
  p[5] = f6;
  p[6] = f7;
  //void doConsenus(char * datafile, char ** setfiles, int count)
  doConsensus("juv_shrimp_errors", p, 7);
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

// main function
int main(int argc, char ** argv)
{
  int l;
  int i = 0;

  if(dolog)
  for(i = 0; i < argc; i ++)
    {
      printf("Argument %d is %s\n", i, argv[i]);
    }

  if(argc <= 1)
    {
      showusage();
      testConsensus();
    }

  /*  else if(strcmp(argv[1], "test") == 0)
    {
      printf("testing mode\n");
    }
  
  else if(strcmp(argv[1], "sibs") == 0)
    {
      printf("sibs mode\n");
      }*/
  else if(strcmp(argv[1], "fathers")== 0 || strcmp(argv[1], "father") == 0)
    {
      if(argc < 5)
        {
          showusage();
          exit(1);
        }

      nloci = atoi( argv[2]);
      mothersmode = 1;
      init();
      readData(argv[3]);
      initMother(); 
     //SetAssigner a(loci);
      if(argc >=6)
        {
          if(strcmp(argv[5],"on") == 0)
            {
              dolog = 1;
            }
          else{
            dolog = 0;
          }
          if(argc>=7)
            {
              readPossibilities(argv[6]);
            }
          else
            {
              readPossibilities("siblings-complete.csv");
            }
        }
      else
        {
          dolog = 0;
          readPossibilities("siblings-complete.csv");
        }
      if( NULL == (dumpfile = fopen( argv[4], "w")))
        {
          printf("Unable to write results\n");
          exit(-1);
        }
      sortLociByAlleleFrequency();
      reconstruct();


      writeResultsFile(argv[4]);
      cleanUp();

    }
  else if(strcmp(argv[1], "test") == 0)
    {

      if(argc < 5)
	{
	  showusage();
	  exit(1);
	}
      
      nloci = atoi( argv[2]);
      init();
      readData(argv[3]);
      //SetAssigner a(loci);
      if(argc >=6)
	{
	  if(strcmp(argv[5],"on") == 0)
	    {
	      dolog = 1;
	    }
	  else{
	    dolog = 0;
	  }
	  if(argc>=7)
	    {
	      readPossibilities(argv[6]);
	    }
	  else
	    {
	      readPossibilities("siblings-complete.csv");
	    }
	}
      else
	{
	  dolog = 0;
	  readPossibilities("siblings-complete.csv");
	}
      if( NULL == (dumpfile = fopen( argv[4], "w")))
	{
	  printf("Unable to write results\n");
	  exit(-1);
	}
      sortLociByAlleleFrequency();
      reconstruct();

      writeResultsFile(argv[4]);
      char gamsname[512];
      strcpy(gamsname, argv[4]);
      strcat(gamsname, ".gms");
      writeGamsFile(gamsname,  sibgroupsmap);
      cleanUp();
      /*readPossibilities(argv[3]);
			  readSlashyData(argv[4]);
			  assignSets();
			  writeResultsToFile(argv[5]);*/
    }
  
  /** 
   * Sibs mode reads actual sibling groups from data and turn by turn
   * gives each group to the setassigner so that we know all the
   * possibilities and can calculate the distance for each possibility
   * 
   */
  else if(argc > 1 && (strcmp(argv[1],"sib") == 0 || strcmp(argv[1],"sibs")==0))
    {
      if(argc < 8)
	{
	  showusage();
	  exit(1);
	}
      if( (strcmp(argv[7],"on") == 0))
	{
	  dolog = 1;
	}
      else
	{
	  dolog = 0;
	}
      nloci = atoi( argv[2]);
      //			doSibsAnalysis(nloci, log, argv[3],argv[5], argv[4], argv[6]	);
      
    }
  else if(argc > 1 && strcmp(argv[1],"consensus") == 0)
    {
      if(argc < 5)
	{	
	  showusage();
	  return 1;
	}
      //      if(argc >= 3)
	{

	  //  if( (strcmp(argv[4],"on") == 0))
	    {
	      dolog = 1;
	    }
	  
	}
      nloci = atoi( argv[2]);
      int minsets = atoi(argv[3]);
      if(nloci == 0)
	{
	  printf("Erroneous # loci\n");
	  exit(-1);
	}
      /*      if(minsets == 0)
	{
	  printf("Erroneous # minsets\n");
	  exit(-1);
	  }*/
      init();     
      readCosts(NULL);
      readPossibilities("siblings-complete.csv");

      doConsensus(argv[3],  (argv+4), argc-4);
      //SetAssigner a(loci);
      //readPossibilities(argv[3]);
      //			Tester t(a);
      /*      if(argc >= 6)
	{
	   doConsensus(argv[3] , (argv+4), );
	  
	  //doEnsemble(argv[4], nloci, a, atoi(argv[5]));
	}
      else
	{
	  
	  for(l = 0; l < nloci; l ++)
	    {
	      //	doEnsemble(argv[4], loci, a, l);
	    }
	    }*/

    }
  else if(argc > 1 && strcmp(argv[1],"greedyconsensus") == 0)
    {
      printf("Entering Greedy Consensus mode\n");
      if(argc < 5)
	{	
	  showusage();
	  return 1;
	}
            if(argc >= 3)
	{

	    if( (strcmp(argv[4],"on") == 0))
	    {
	      dolog = 1;
	    }
	  
	}
      nloci = atoi( argv[2]);
      int minsets = atoi(argv[3]);
        if(nloci == 0)
	{
	  printf("Erroneous # loci\n");
	  exit(-1);
	}
	/*      if(minsets == 0)
	{
	  printf("Erroneous # minsets\n");
	  exit(-1);
	  }*/
      init();     
      readCosts(NULL);
      readPossibilities("siblings-complete.csv");

      doGreedyConsensus(argv[3], (argv+4), argc-4);
      //SetAssigner a(loci);
      //readPossibilities(argv[3]);
      //			Tester t(a);
      /*      if(argc >= 6)
	{
	   doConsensus(argv[3] , (argv+4), );
	  
	  //doEnsemble(argv[4], nloci, a, atoi(argv[5]));
	}
      else
	{
	  
	  for(l = 0; l < nloci; l ++)
	    {
	      //	doEnsemble(argv[4], loci, a, l);
	    }
	    }*/

    }
  
  else if(argc > 1 && strcmp(argv[1],"autoconsensus") == 0)
    {
      if(argc < 3)
        {
          showusage();
          return 1;
        }

      if(argc >= 5)
	{
	  
          if( (strcmp(argv[4],"on") == 0))
	    {
	      dolog = 1;
	    }
	  
	}

      nloci = atoi( argv[2]);
      init();
      readCosts(NULL);
      readPossibilities("siblings-complete.csv");
      doAutoConsensus(argv[3], nloci);
      //SetAssigner a(loci);
      //readPossibilities(argv[3]);
      //                        Tester t(a);
      /*      if(argc >= 6)
        {
           doConsensus(argv[3] , (argv+4), );

          //doEnsemble(argv[4], nloci, a, atoi(argv[5]));
        }
      else
        {

          for(l = 0; l < nloci; l ++)
            {
              //        doEnsemble(argv[4], loci, a, l);
            }
            }*/

    }
  
  else if(argc > 1 && strcmp(argv[1],"autogreedyconsensus") == 0)
    {
      if(argc < 3)
        {
          showusage();
          return 1;
        }

      if(argc >= 5)
	{
	  
          if( (strcmp(argv[4],"on") == 0))
	    {
	      dolog = 1;
	    }
	  
	}

      nloci = atoi( argv[2]);
      init();
      readCosts(NULL);
      readPossibilities("siblings-complete.csv");
      doAutoGreedyConsensus(argv[3], nloci);
      //SetAssigner a(loci);
      //readPossibilities(argv[3]);
      //                        Tester t(a);
      /*      if(argc >= 6)
        {
           doConsensus(argv[3] , (argv+4), );

          //doEnsemble(argv[4], nloci, a, atoi(argv[5]));
        }
      else
        {

          for(l = 0; l < nloci; l ++)
            {
              //        doEnsemble(argv[4], loci, a, l);
            }
            }*/

    }
  else{
    showusage();
  }
  getrusage(RUSAGE_SELF,&usage );
  dumpusage("usage");

  
  return 0;
}
