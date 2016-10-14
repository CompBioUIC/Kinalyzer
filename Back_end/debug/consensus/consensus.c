
/****************************************************************************
 * consensus.c
 * Author: Saad Sheikh
 *
 * Contains definitions of all methods pertaining to consensus
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


#include "hashtable.h"
#include "hashtable_itr.h"
#include "structures.h"
#include "main.h"
#include "utilities.h"


double getAssignmentCost(sibgroup * group, individual * ind);

double getAssignmentBenefit(sibgroup * group, individual * ind);

void blank(void*);

int getAssignmentBasedGroupHash(sibgroup * group);

int equalsAssignmentGroups(sibgroup * group1, sibgroup * group2);

struct hashtable * allconsensusgroups;

//void blanko(void *p){}

/****************************************************************************
 * Read Costs from file  
 *
 ***************************************************************************/
void readCosts(const char * filename)
{
  const char * constfilename = "costs.txt";

  if(filename == NULL)
    {
      filename =constfilename;
    }
  FILE * infile = fopen(filename, "r");
  char buff[512];
  char * read ;

  read = fgets(buff, 512, infile);
  while( infile && read != NULL)
    {
      char * cost = buff+1;
      double cvalue = atof(cost);
      switch(buff[0])
	{
	case '#':
	  break;
	case 'n':
	  costs[NULL_ALLELE_ERROR]=cvalue;
	  break;
	case 'm':
	  costs[HOMO_MISTYPE_ERROR]=cvalue;
	  break;
	case 't':
	  costs[ HETERO_MISTYPE_ERROR ]=cvalue;
	  break;
	case 'p':
	  costs[POSSIBILITY_MISFIT]=cvalue;
	  break;
	case 'g':
	  costs[MAX_GROUP_EDIT]=cvalue;
	  break;
	case 'e':
	  costs[MAX_ERROR_RATE]=cvalue;
	  break;
	case 'x':
	  maxedit=cvalue;
	  break;
	default:
	  break;
	}

  read = fgets(buff, 512, infile);
    }
}

double getMergeCost(sibgroup * group1, sibgroup * group2)
{
  double cost = 0.0;
  double bcost = 0.0;
  sibgroup * merge = copySibgroup(group1);
  node * index; 
  node * indi;
  int smoothmerge = 1;
  int icost = 0;
  indi = data.head;
  int totalinds = group1->assignments->count + group2->assignments->count ;
  for(index = group2->assignments->head; index != NULL && cost <= costs[MAX_GROUP_EDIT] && cost <= ceil(costs[MAX_ERROR_RATE] *totalinds);
      index = index->next)
    {
      for( ;((individual*)indi->data)->index != index->data && indi != NULL; indi =indi->next)
	{
	  // nthn
	}
        if(smoothmerge)
	{

	  //	  sibgroup * mergeCopy = copySibgroup(merge);
	  if(assignIndividualToGroup(merge, indi->data) >0 )
	    {
	      bcost += getAssignmentBenefit(merge, indi->data);
	      //	      deleteSibgroup(mergeCopy);
	    }
	  else
	    {
	      // deleteSibgroup(mergeCopy);
	      deleteSibgroup(merge);
	      merge = copySibgroup(group1);
	      smoothmerge = 0;
	      icost  = getAssignmentCost(merge, indi->data);
	      if(icost <= maxedit)
		cost += icost  ;
	      else 
		cost += INFINITY;
	      
	    }
	  //	  deleteSibgroup(mergeCopy);
	}
       else
	{
	  icost = getAssignmentCost(merge, indi->data);
	  if(icost <= maxedit)
	    cost += icost  ;
	  else
	    cost += INFINITY;

	}
    }
  deleteSibgroup(merge);
  if(smoothmerge)
    return  bcost;
  else
    return cost;
}

/*********************************************************************************
 * Calculates assignment distance between a sibgroup and an individual
 *
 * *******************************************************************************/
double getAssignmentCost(sibgroup * group, individual * indi)
{
	double cost = 0.0;
	int locus, loc;
	for(loc = 0; loc < nloci && cost <= maxedit; loc ++)
	{
	  locus = lociorder[loc];
	  // possibilities = (*group).possibilitysets[locus];
	  struct hashtable * allelemappings = group->intmaps[locus];
	  Pair mappedpair;
	  int *xlkup, *ylkup;
	  xlkup= hashtable_search(allelemappings, & indi->loci[locus].x);
	  ylkup = hashtable_search(allelemappings, & indi->loci[locus].y);
	  if(NULL == xlkup
	     && NULL == ylkup)
	    {
	      if(indi->loci[locus].x == indi->loci[locus].y)
		{
		  cost += costs[HOMO_MISTYPE_ERROR];

		  if(dolog)
		    {
		      fprintf(logfile, "Error seems to be at locus %d with 2ble allele: %d", locus,   indi->loci[locus].x);
		    }

		}
	      else
		{			      
		  cost+= costs[HETERO_MISTYPE_ERROR] * 2;

		  if(dolog)
		    {
		      fprintf(logfile, "Error seems to be at locus with alleles: %d and %d\n", locus,  indi->loci[locus].x, indi->loci[locus].y);
		    }

		}
	    }
	  else if(xlkup == NULL)
	    {
	      if(dolog)
		{
		  fprintf(logfile, "Error seems to be at locus %d with allele: %d\n", locus,  indi->loci[locus].x);
		}
	      cost += costs[HETERO_MISTYPE_ERROR];
	    }
	  else if (ylkup == NULL)
	    {
              if(dolog)
                {
                  fprintf(logfile, "Error seems to be at locus %d with allele: %d\n", locus, indi->loci[locus].x);
                }

	      cost += costs[HETERO_MISTYPE_ERROR];

	    }
	  else
	    {
	      Pair p;
	      p.x = *xlkup;
	      p.y = *ylkup;
	      int * lkuppossibilities = lookupPossibilities( & p);
	      if(lkuppossibilities == NULL)
		{
		  lkuppossibilities = EMPTY_POSSIBILITIES;
		}
	      int * intersection = intersectPossibilities(group->possibilitysets[locus], lkuppossibilities);
	      if(isEmptyIntersection(intersection))
		{
		  cost += costs[POSSIBILITY_MISFIT];
		

		  if(dolog)
		    {
		      fprintf(logfile, "Error seems to be at locus %d with allele combinations, 4-allele property is satisfied, but 2-allele is not\n" , locus );
		    }
		}
	      free(intersection);
	    }
	}
	return cost;
      
}



/**********************************************
 * Assumes assignment is possible
 **********************************************/
double getAssignmentBenefit(sibgroup * group, individual * indi)
{
  double cost = -(costs[HETERO_MISTYPE_ERROR]*2+costs[POSSIBILITY_MISFIT])*nloci;
  int locus, loc;
  for(loc = 0; loc < nloci ; loc ++)
    {
      locus = lociorder[loc];
      // possibilities = (*group).possibilitysets[locus];
      struct hashtable * allelemappings = group->intmaps[locus];
      Pair mappedpair;
      int *xlkup, *ylkup;
      xlkup = hashtable_search(allelemappings, & indi->loci[locus].x);
      ylkup = hashtable_search(allelemappings, & indi->loci[locus].y);
      if(NULL == xlkup
	 && NULL == ylkup)
	{
	  if(indi->loci[locus].x == indi->loci[locus].y)
	    {
	      cost += costs[HOMO_MISTYPE_ERROR];
	    }
	  else
	    {			      
	      cost+= costs[HETERO_MISTYPE_ERROR]*2;
	    }
	}
      else if(xlkup == NULL|| ylkup == NULL)
	{
	  cost += costs[HETERO_MISTYPE_ERROR];
	}
      else
	{
	  Pair p;
	  p.x = *xlkup;
	  p.y = *ylkup;
	  int * lkuppossibilities = lookupPossibilities( & p);
	  if(lkuppossibilities == NULL)
	    {
	      lkuppossibilities = EMPTY_POSSIBILITIES;
	    }
	  int * intersection = intersectPossibilities(group->possibilitysets[locus], lkuppossibilities);
	  if(equalsPossibilitySets(intersection, group->possibilitysets[locus]))
	    {
	      cost += costs[POSSIBILITY_MISFIT];
	    }
	  /*	  if(isEmptyIntersection(intersection))
	    {
	      cost += costs[POSSIBILITY_MISFIT];
	      }*/
	  free(intersection);
	}
    }
  return cost;
      
}


/***********************************************************************************
 * Reads the solution set file
 * returns a pointer to a new list file read from the solution file
 *
 ***********************************************************************************/
list * readSetFile(const char * filename)
{	
  FILE * setfile;
  if(NULL == (setfile = fopen( filename, "r")))
    {
      printf("Error can't open file for reading sets %s\n", filename);
      exit(-1);
    }
	
  list * sets= malloc(sizeof(list));
  sets->head = NULL;
  sets->count = 0;
  char * read;
  //  char buff[512];
  char * pbuff = calloc(512, sizeof(char)); //buff;
  int id = 0;
  read = fgets(pbuff, 512, setfile);
  while( setfile && read != NULL)
    {
      sibgroup * itemset= malloc(sizeof(sibgroup));
      initializeSibgroup(itemset);
      itemset->mutationstamp = id++;
      
      
      if(strlen(pbuff) == 0)
	{
	  continue;
	}
      char *name, *ind;
      name = strtok(pbuff, " \t");
      
      /* Extract remaining 
       * strings 		*/
      int indno = 0;
      while ( (ind=strtok(NULL, " \t")) != NULL)
	{
	  //printf("%s\n", ind);
	  indno = atoi(ind);
	  addToSortedList( itemset->assignments, indno );
	}
      addToList( sets, itemset);
      //sets.push_back(itemset);
      read = fgets(pbuff, 512, setfile);
    }
  free(pbuff);
  return sets;

}

/****************************************************************************
 * populates solutions
 *
 ***************************************************************************/
void populateAssignmentSets(list * sets)
{
  node * ptr;
  ptr= sets->head;
  for( ; ptr != NULL ; ptr = ptr->next)
    {
      sibgroup * group = (sibgroup* )ptr->data;
      node * iptr;
      iptr = group->assignments->head;
      data.current = data.head;
      int icount = 0;
      for( ; iptr != NULL; iptr = iptr->next)
	{
	  for(; ((individual*)data.current->data)->index < iptr->data && data.current!= NULL; icount ++)
	    {
	      data.current = data.current->next;
	    }
	  int result = assignIndividualToGroup(group, data.current->data);
	  if(result == 0)
	    {
	      printf("In Set: ");
	      node * printer = group->assignments->head;
	      for( ; printer != NULL; printer = printer->next)
		{
		  printf(" %d", printer->data);
		}
	      printf("\n");
	      printf("Error: unable to assign to known sib group: %d\n",((individual*) data.current->data)->index);
	      exit(-1);
	    }
	}
    }
  
}

/***************************************************************************
 * returns final list of sets
 * two inds are in same set IFF everyone agrees they should be
 *
 ***************************************************************************/
list vote(list * setlists, int votecount)
{
  list tuple;
  tuple.head = NULL;
  tuple.count = 0;
  list * final = malloc(sizeof(list));
  final->head = NULL;
  final->count = 0;
  int ** votes = calloc(data.count, sizeof(int*));
  int i, j, k;
  node * p1, *p2;
  int * v4 = calloc(data.count, sizeof(int));
  sibgroup * p3;
  list * disputed = malloc(sizeof(list));
  disputed->head = NULL;
  disputed->count = 0;
  for(i = 0;i < data.count; i++)
    {
      votes[i] = calloc(data.count, sizeof(int));
    }
  for(p1 = setlists->head;p1 != NULL; p1 = p1->next)
    { // each setlist
      printf("new soln\n");
      int ** v2 = calloc(data.count, sizeof(int*));
      for(i = 0; i < data.count; i ++)
	{
	  v2[i] = calloc(data.count, sizeof(int));
	}
      int * v3 = calloc(data.count, sizeof(int));

    for(p2 = ((list*)p1->data)->head ; p2 != NULL;p2=p2->next )
      { // each set
	p3 = p2->data;
	list * assignments=p3->assignments;
	node * pp1,* pp2;

	//	printf("Examining New set\n");

	for(pp1 = assignments->head; pp1 != NULL; pp1 = pp1->next)
	  {
	    int x = pp1->data;
	    if(v3[x]== 0) v3[x]= 1;
	    else v3[x] = 2;
	  for ( pp2 =  pp1->next ; pp2 != NULL;pp2 = pp2->next )
	    {// every set n other set
	      /*	      int ** v2= calloc(data.count, sizeof(int*));

	      for(i = 0;i < data.count; i++)
		{
		  v2[i] = calloc(data.count, sizeof(int));
		  }*/	      
	      //	      int x = pp1->data;
	      int y = pp2->data;

	      if( v2[x][y] == 1)
		{
		  votes[x][y] --;
		  v2[x][y] = 2;
		}
	      else if(v2[x][y] == 0)
		{
		  votes[x][y] ++;
		  v2[x][y] = 1;
		}
	      /*
	      for(i = 0;i < data.count; i++)
		{
		  free(v2[i]);// = calloc(data.count, sizeof(int));
		}	      
	      free (v2);*/
	    }

	  }	
      }
	for(i = 0;i < data.count; i++)
	  {
	    free(v2[i]);// = calloc(data.count, sizeof(int));
	    if(v3[i] == 2)
	      {
		v4[i] = 1;
		for(j = 0; j < data.count; j ++)
		  //for(j = 0; j < data.count ; j ++)
		    {
		      votes[i][j] --;
		      votes[j][i] --;
		    }
	      }
	  }
	free (v2);
	free(v3);
	
      
    }



  int * itemset = calloc(data.count, sizeof(int));
  for(j = 0; j < data.count; j ++ )
    {
      itemset[j] = j;
    }
  int * freebies = calloc(data.count, sizeof(int));

  for( j = 0; j < data.count; j ++ )
    {
      freebies[j] = 1;
    }
  for( j = 0; j < data.count; j ++ )   
    {
      for( k = j + 1 ; k < data.count; k++)
	{
	  if(votes[j][k] >= votecount)
	    {
	      freebies[j] = freebies[k] = 0;
	      itemset[k] = itemset[j];
	    }
	}
    }


  list * temp = calloc(data.count, sizeof(list));
  for(j = 0; j <data.count ; j ++)
    {
      temp[j].head= NULL;
      temp[j].count = 0;
    }
  node * n = data.head;
  for(j = 0, n = data.head ; j < data.count && n != NULL; j ++, n = n->next)
    {
      if(j ==4)
	printf("");
      if(!freebies[j])
	addToSortedList(& temp[itemset[j]], j);
      else
	{
	 
	  //	  addToList(disputed, n->data);
	}
    }
  for(j = 0 ; j < data.count; j ++)
    {
      if(temp[j].count > 1)
	{
	  sibgroup * group = malloc(sizeof(sibgroup));
	  initializeSibgroup(group);

	  group->assignments->head = temp[j].head;
	  group->assignments->count = temp[j].count;
	  addToList(final, group);
	}
      else if (temp[j].count == 1 || freebies[j])
	{
	int k;
	for(k = 0, n = data.head ; k < j && n != NULL; k ++, n = n->next)
	  {
	  }
	//	addToList(disputed, n->data);
	sibgroup * group = malloc(sizeof(sibgroup));
	initializeSibgroup(group);
	group->assignments->head = malloc(sizeof(node));
	group->assignments->head->data= j;
	group->assignments->head->next = NULL;
	group->assignments->head->previous = NULL;
	group->assignments->count = 1;

	addToList(final, group);
      }
      
    }
  /*  cleanup    */
  free(temp);
  for(i = 0;i < data.count; i++)
    {
      free(votes[i]);// = calloc(data.count, sizeof(int));
    }	      
  free(votes);
  free(v4);
  free(itemset);
  addToList(&tuple, final);
  addToList(&tuple, disputed);
  deleteList(disputed, blank);
  return tuple;
}

/************************************************************************
 * main method for consensus
 *
 ************************************************************************/
void doConsensus(char * datafile,  char ** setfiles, int count)
{
  int i = 0;
  //  init();

  if(dolog == 1)
    {
      char name[100];
     strcpy(name, datafile);
      strcat(name, ".log");
      logfile = fopen(name, "w");
    }
  nloci = count;
  readData(datafile);
  sortLociByAlleleFrequency();
  list setlistslist;
  setlistslist.head = NULL;
  setlistslist.count = 0;
  for( ;i < count; i++)
    {

      list * sets = readSetFile(setfiles[i]);
      //    populateAssignmentSets( sets);
      addToList(&setlistslist, sets);
    }
  list result = vote(&setlistslist, count);
  list * sets = result.head->data;
  populateAssignmentSets(sets);
  list * disputed = result.head->next->data;

  /*  mergeGroups(sets);
  printResults("preresults",sets);
  assignDisputed(sets, disputed);
  mergeGreedyGroups(sets, minsets);
  printResults("results", sets);*/
  
  generateAllConsensusGroups(sets);
  printWeightedResultsGamsFile(datafile);  

  if(dolog)
    fclose(logfile);
}

/************************************************************************
 * main method for consensus
 *
 ************************************************************************/
void doGreedyConsensus(char * datafile, char ** setfiles, int count)
{
  int i = 0;
  //  init();

  if(dolog == 1)
    {
      char name[100];
     strcpy(name, datafile);
      strcat(name, ".log");
      logfile = fopen(name, "w");
    }
  nloci = count;
  readData(datafile);
  sortLociByAlleleFrequency();
  list setlistslist;
  setlistslist.head = NULL;
  setlistslist.count = 0;
  for( ;i < count; i++)
    {
      list * sets = readSetFile(setfiles[i]);
      //    populateAssignmentSets( sets);
      addToList(&setlistslist, sets);
    }
  list result = vote(&setlistslist, count);
  list * sets = result.head->data;
  populateAssignmentSets(sets);
  list * disputed = result.head->next->data;

  mergeGreedyGroups(sets);
  //  printResults("preresults",sets);
  // assignDisputed(sets, disputed);
  //  mergeGreedyGroups(sets, minsets);
  char buff[512];
  strcpy(buff, "consensus_");
  strcat(buff, datafile);
  printResults(buff, sets);
  
  /*  generateAllConsensusGroups(sets);
      printWeightedResultsGamsFile(datafile);  */

  if(dolog)
    fclose(logfile);
}

/***************************************************************

 ************************************************************/
void assignWithEdit(sibgroup * group, individual * indi)
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
	      // mappedpair.x = adjustAlleles(group, locus, indi->loci[locus].x);
	      
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
	    // failed = 1;
	    // break;
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
	      //	      mappedpair.y = adjustAlleles(group, locus, indi->loci[locus].y);
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
	    //  failed = 1;
	    // break;
	  }
	}else{
	mappedpair.y = * mappedvalue;
      }
	  // possibilities

      int * lookeduppossibilities = lookupPossibilities(& mappedpair);
      if(lookeduppossibilities == NULL)
	{
	  lookeduppossibilities = EMPTY_POSSIBILITIES;
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
	  //failed = 1;
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


/********************************
 * Prints Results
 *******************************/
void printResults(const char * filename, list * sets)
{
  FILE * results ;
  if(NULL == (results = fopen(filename, "w")))
    {
      printf("Unable to open output file %s\n", filename);
      exit(-1);
    }
  node * p = sets->head;
  int setcounter = 0;
  for( ; p != NULL; p = p->next)
    {
      sibgroup * set = p->data;
      int counter = 0;
      node * indnode = set->assignments->head;
    
      fprintf(results, "\"Set(%d):\"", setcounter++/*, set->edit*/);
      //  fprintf(results, "%4.2f", set->edit);
      //      individual * ind =indnode->data;
      for(counter = 0; counter < data.count; counter ++)
	{

	  if(indnode != NULL && counter == indnode->data)
	    {
	      indnode = indnode->next;
	
	      //fprintf(results, "\t1");
	      fprintf(results, " %d",counter);
	    }
	  else
	    {
	      //fprintf(results, "\t0");
	    }
	 
	}
      printf("\n");
      fprintf(results, "\n");
    }
  //  flush(results);
  fclose(results);
}

/***********************
 * 
 **********************/
void assignDisputed(list * sets, list * disputed)
{
  node * p;
  int icount = 0;
  int scount =0;
  for(p = disputed->head; p != NULL; p = p->next, icount ++)
    {
      individual * jones = p->data;
      node * q;
      double mincost = 99999999;
      node * assignment;
      sibgroup * assignedGroup;
      for(q = sets->head, scount = 0; q !=NULL ; q= q->next, scount ++ )
	{

	  double cost = 0.0;
	  sibgroup * g = copySibgroup(q->data);
	  if(assignIndividualToGroup(g, p->data))
	    {
	      cost = getAssignmentBenefit(q->data, p->data);
	    }
	  else
	    {
	      cost = getAssignmentCost(q->data, p->data);
	    }
	  if(dolog)
	    {
	      fprintf(logfile, "Assignment Cost for individual:%d to %d is %1.4f \n", jones->index, scount, cost);
	      printf("Assignment Cost for individual:%d to %d is %1.4f \n", jones->index, scount, cost);
	    }
	     //	  double 
	  deleteSibgroup(g);

	  if(cost < mincost )

	    {
	      mincost = cost;
	      assignedGroup = (sibgroup*)q->data;
	      assignment = assignedGroup->assignments;
	    }

	  /*	  if((cost + ((sibgroup*)q->data)->edit) < costs[MAX_GROUP_EDIT])
	  {	 
	     sibgroup * copy = copySibgroup(q->data);
	     addToList(sets, copy);
	     assignWithEdit(copy, p->data);
	     copy->edit += mincost;
	     addToSortedList(copy->assignments,((individual*) p->data)->index);
	     }*/
	}

      if(mincost <= maxedit )
	{
	  /*  sibgroup * copy = copySibgroup(assignedGroup);
	  addToList(sets, copy);
	  assignWithEdit(copy, p->data);
	  copy->edit += mincost;
	  addToSortedList(copy->assignments,((individual*) p->data)->index);*/
	  assignWithEdit(assignedGroup, p->data);
	  addToSortedList(assignment, ((individual*) p->data)->index);
	}

      else
	
	{
	  sibgroup *singleton = malloc(sizeof(sibgroup));
	  initializeSibgroup(singleton);
	  assignIndividualToGroup(singleton, p->data);
	  addToSortedList(singleton->assignments, ((individual*) p->data)->index);
	  addToList(sets, singleton);
	  
	  }

    }
}

double max(double x, double y)
{
  return (x>y ? x: y);

}

void mergeGreedyGroups(list * groups)
{
  node * group1;
  node * group2;
  node * opt1, *opt2;
  group1 = groups->head;

  double optcost = 9999999;
  int merging = 0;
  do{
    merging = 0;
    opt2 = opt1 = NULL;
    optcost = 9999999;
    group1 = groups->head;
    int totalinds;
    for( ; group1!= NULL; group1 = group1->next)
      {
	for(group2 = group1->next; group2 != NULL; group2 = group2->next )
	  {
	    sibgroup *gp1 = group1->data;
	    sibgroup * gp2 = group2->data;
	    double mcost = getMergeCost(gp1, gp2);
	    double mcost2 = getMergeCost(gp2, gp1);
	    if(mcost < optcost)
	      {
		opt1 = group1;
		opt2 = group2;
		optcost = mcost +  gp1->edit;
		totalinds = gp1->assignments->count + gp2->assignments->count;
	      }
	    if (mcost2 < optcost)
	      {
		
		opt1 = group1;
		opt2 = group2;
		optcost = mcost2 +  gp2->edit;
		totalinds = gp1->assignments->count + gp2->assignments->count;
	      }
	   
	  }
      }
    if(optcost <= costs[MAX_GROUP_EDIT] && optcost <= ceil(costs[MAX_ERROR_RATE] * totalinds))
      {
	sibgroup * merge = copySibgroup(opt1->data);
	node * iptr = data.head;
	if(optcost >= merge->edit)
	  merge->edit = optcost;
	int i= 0;
	node * ptr;
	for(ptr = ((sibgroup*) opt2->data)->assignments->head ; ptr != NULL ; ptr = ptr->next)
	  {

	    
	    if(optcost < merge->edit)
	      {
		for(; iptr != NULL && i != ptr->data; iptr= iptr->next, i ++);
		assignIndividualToGroup(merge, iptr->data);
	      }
	    addToSortedList(merge->assignments, ptr->data);
	  }


	node *mnode= malloc(sizeof(node));
	mnode->next = opt1->next;
	mnode->previous = opt1->previous;
	mnode->data = merge;
	
	if(groups->head == opt1)
	  groups->head = mnode;
	
	if(opt1->previous != NULL)
	  opt1->previous->next = mnode;
	opt1->next->previous = mnode;
	deleteSibgroup(opt1->data);
	
	free(opt1);
	//opt1 = mnode;
	
	node * temp = opt2->next;
	opt2->previous->next = opt2->next;
	if(opt2->next)
	  opt2->next->previous = opt2->previous;
	//opt2 = temp;
	deleteSibgroup(opt2->data);
	free(opt2);
	groups->count --;
	merging = 1;
      }
  }while( merging);
}


void mergeGroups(list * groups)
{
  node * group1;
  node * group2;
  group1 = groups->head;
  int merging = 0;
  do{
    merging = 0;
  for( ; group1!= NULL; group1 = group1->next)
    {
      for(group2 = group1->next; group2 != NULL; )
	{
	  sibgroup * merge = copySibgroup(group1->data);
	  sibgroup * g = group2->data;
	  int giveup = 0;
	  int i = 0;
	  node * ind;
	  node * assignedind;
	  for ( ind = data.head, assignedind = g->assignments->head; assignedind != NULL && i < individuals && ! giveup; ind = ind->next, i ++)
	    {
	      if(assignedind->data == i)
		{
		  int result = assignIndividualToGroup(merge, ind->data);
		  addToSortedList(merge->assignments, i);
		  if(result == 0)
		    giveup = 1;
		  assignedind = assignedind->next;
		}
	    }
	  if(!giveup)
	    {
	      ///	      addToList(groups, merge);
	      node *mnode= malloc(sizeof(node));
	      mnode->next = group1->next;
	      mnode->previous = group1->previous;
	      mnode->data = merge;

	      if(groups->head == group1)
		groups->head = mnode;

	      if(group1->previous != NULL)
		group1->previous->next = mnode;
	      group1->next->previous = mnode;
	      deleteSibgroup(group1->data);
	      
	      free(group1);
	      group1 = mnode;

	      node * temp = group2->next;
	      group2->previous->next = group2->next;
	      if(group2->next)
		group2->next->previous = group2->previous;
	      group2 = temp;
	      groups->count --;
	      merging = 1;
	    }
	  else
	    {
	      deleteSibgroup(merge);
	      group2 = group2->next;
	    }
	}
    }
  }while(merging);
}

void printWeightedResultsGamsFile(const char * filename)
{
  /*function output = makegams(inname, adat)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
[m,n] = size(adat);*/
  char inname[512];
  char outname[512];
  strcpy(inname, filename);
  strcat(inname,".gms");
  FILE * fid = fopen(inname,"w");
  int n = data.count;
  int i, j;
  int m =  hashtable_count(allconsensusgroups);
  fprintf(fid, "$ONEMPTY \n \n");
  fprintf(fid, "SET i   sibling   /s1*s%d/; \n",n);
  fprintf(fid, "SET j   sibset    /t1*t%d/; \n \n",m);
  fprintf(fid, "TABLE A(j,i)    sets covered by node i \n");
  fprintf(fid, "     ");
  for (i = 1 ; i <= n; i ++)
    {
      fprintf(fid, "\t s%d",i);
    }
  fprintf(fid, " \n");
  
  struct hashtable_iterator * iter ;

  //  struct hashtable_iterator * itr1, * itr2;
  iter = hashtable_iterator(allconsensusgroups); 
  
  for( i=1; i <= m; i ++)
   {
     sibgroup * group = hashtable_iterator_key(iter);
     fprintf(fid, "t%d",i);
     for(j = 1 ; j <= n; j ++)
       fprintf(fid, "\t %d",searchList(group->assignments, j - 1));
     
     fprintf(fid, " \n");
     if(!hashtable_iterator_advance(iter))
       break;
   }
 free(iter);
 fprintf(fid, "; \n \n");

 fprintf(fid, "SET k   weight    /u1*u1/; \n");
 
 fprintf(fid, "TABLE B(j,k)    weight of set \n");

 fprintf(fid, "\t u1\n");
 
  iter = hashtable_iterator(allconsensusgroups); 
 for( i=1; i <= m; i ++)
   {
     sibgroup * group = hashtable_iterator_key(iter);
     fprintf(fid, "t%d\t %2.3f\n", i, group->edit);
     hashtable_iterator_advance(iter);
   }
 free(iter);
 fprintf(fid, ";\n");
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
fprintf(fid, "        coveredby(i)\n");
fprintf(fid, "        coveringcosteqn\n");
fprintf(fid, "        overlapeqn\n");
fprintf(fid, "        setcov(i) \n");
fprintf(fid, "	           objective \n");
fprintf(fid, "; \n");

fprintf(fid, "VARIABLE TF; \n");
fprintf(fid, "VARIABLES\n");
fprintf(fid, "        covering(i) number of sets covering an individual\n");
fprintf(fid, "        overlap\n");
fprintf(fid, "        coveringcost\n");
fprintf(fid, ";\n");

fprintf(fid, "coveredby(i)..\n");
fprintf(fid, "        covering(i) =e= sum(j, A(j,i)*xset(j));\n");
fprintf(fid, " \n");
fprintf(fid, "setcov(i).. \n");
fprintf(fid, "        covering(i) =e= 1; \n");
fprintf(fid, " \n");
fprintf(fid, "* Objective Function \n");
fprintf(fid, "objective.. \n");
fprintf(fid, "TF =e= sum(j, xset(j) *  B(j,k)); \n");
fprintf(fid, " \n");
fprintf(fid, "***** Model Definition ****** \n");
fprintf(fid, "MODEL setcover / ALL /; \n");
fprintf(fid, "***** Solution Options ******* \n");
fprintf(fid, "OPTION lp=cplex; \n");
fprintf(fid, "OPTION mip=cplex; \n");
fprintf(fid, "OPTION optcr=0.00001; \n");
fprintf(fid, "OPTION optca=0.00001; \n");
fprintf(fid, "OPTION iterlim=50000; \n");
fprintf(fid, "OPTION reslim=500; \n");
fprintf(fid, "option rmip = cplex; \n");
fprintf(fid, "SOLVE setcover using mip minimizing TF; \n \n");
fprintf(fid, "DISPLAY xset.l; \n");
fprintf(fid, "DISPLAY overlap.l;\n");
fclose(fid);
//output = 0;


}

void doAutoConsensus(const char * filename, int locicount)
{
  if(dolog == 1)
    {
      char name[100];
      strcpy(name, filename);
      strcat(name, ".log");
      logfile = fopen(name, "w");
    }
  nloci = locicount - 1;
  char ** filenames = calloc(locicount, sizeof(char*));
  //
  //  readData(filename);
  list source = data;
  int droppedlocus = 0;
  for( droppedlocus = 0; droppedlocus < locicount; droppedlocus ++)
    {
      init();

      char solname[90];
      strcpy(solname, filename);
      strcat(solname, "_set");
      char lstr[4];
      sprintf(lstr, "%d", droppedlocus);
      strcat(solname, lstr);
      char gmsname[94];
      strcpy(gmsname, solname);
      strcat(gmsname, ".gms");
      // dropLocus(droppedlocus, source);
      readSelectedData(filename, droppedlocus);
      readPossibilities("siblings-complete.csv");     
      sortLociByAlleleFrequency();
      reconstruct();
      
      dumpfile = fopen(solname, "w");
      writeResultsFile(solname);
      writeGamsFile(gmsname, sibgroupsmap);
      deleteList(&data, datacleaner);
      cleanUp();
      char gamscmd[512];
      strcpy(gamscmd, "gams ");
      strcat(gamscmd, gmsname);
      system(gamscmd);
      char solncmd[512];
      char lstname[94];
      strcpy(lstname, solname);
      strcat(lstname, ".lst");
      strcpy(solncmd, "extract-solution-param.pl ");
      strcat(solncmd, lstname);
      system(solncmd);
      
      char * solnname = calloc(512, sizeof(char));
      strcpy(solnname, solname);
      strcat(solnname, ".soln");
      filenames[droppedlocus] = solnname;
      
    }
  nloci++;
  //  doConsensus(filename, filenames, locicount);
  int i;
  init();
  readPossibilities("siblings-complete.csv");
  readData(filename);
  readCosts(NULL);  
  sortLociByAlleleFrequency();
  list setlistslist;
  setlistslist.head = NULL;
  setlistslist.count = 0;
  for(i=0 ;i < nloci; i++)
    {

      list * sets = readSetFile(filenames[i]);
      //    populateAssignmentSets( sets);
      addToList(&setlistslist, sets);
    }
  list result = vote(&setlistslist, nloci);
  list * sets = result.head->data;
  populateAssignmentSets(sets);
  //list * disputed = result.head->next->data;

  //  mergeGroups(sets);
  //printResults("preresults",sets);
  //  assignDisputed(sets, disputed);
  char outputname[50];
  strcpy(outputname, "consensus_");
  strcat(outputname, filename);
  /* printResults(outputname, sets);
  if(dolog)
  fclose(logfile);*/
  generateAllConsensusGroups(sets);
  printWeightedResultsGamsFile(filename);  

}


void doAutoGreedyConsensus(const char * filename, int locicount)
{
  if(dolog == 1)
    {
      char name[100];
      strcpy(name, filename);
      strcat(name, ".log");
      logfile = fopen(name, "w");
    }
  nloci = locicount - 1;
  char ** filenames = calloc(locicount, sizeof(char*));
  //
  //  readData(filename);
  list source = data;
  int droppedlocus = 0;
  for( droppedlocus = 0; droppedlocus < locicount; droppedlocus ++)
    {
      init();

      char solname[90];
      strcpy(solname, filename);
      strcat(solname, "_set");
      char lstr[4];
      sprintf(lstr, "%d", droppedlocus);
      strcat(solname, lstr);
      char gmsname[94];
      strcpy(gmsname, solname);
      strcat(gmsname, ".gms");
      // dropLocus(droppedlocus, source);
      readSelectedData(filename, droppedlocus);
      readPossibilities("siblings-complete.csv");     
      sortLociByAlleleFrequency();
      reconstruct();
      
      dumpfile = fopen(solname, "w");
      writeResultsFile(solname);
      writeGamsFile(gmsname, sibgroupsmap);
      deleteList(&data, datacleaner);
      cleanUp();
      char gamscmd[512];
      strcpy(gamscmd, "gams ");
      strcat(gamscmd, gmsname);
      system(gamscmd);
      char solncmd[512];
      char lstname[94];
      strcpy(lstname, solname);
      strcat(lstname, ".lst");
      strcpy(solncmd, "extract-solution-param.pl ");
      strcat(solncmd, lstname);
      system(solncmd);
      
      char * solnname = calloc(512, sizeof(char));
      strcpy(solnname, solname);
      strcat(solnname, ".soln");
      filenames[droppedlocus] = solnname;
      
    }
  nloci++;
  //  doConsensus(filename, filenames, locicount);
  int i;
  init();
  readPossibilities("siblings-complete.csv");
  readData(filename);
  readCosts(NULL);  
  sortLociByAlleleFrequency();
  list setlistslist;
  setlistslist.head = NULL;
  setlistslist.count = 0;
  for(i=0 ;i < nloci; i++)
    {

      list * sets = readSetFile(filenames[i]);
      //    populateAssignmentSets( sets);
      addToList(&setlistslist, sets);
    }
  list result = vote(&setlistslist, nloci);
  list * sets = result.head->data;
  populateAssignmentSets(sets);
  //list * disputed = result.head->next->data;

   mergeGreedyGroups(sets);
  //printResults("preresults",sets);
  //  assignDisputed(sets, disputed);
  char outputname[50];
  strcpy(outputname, "consensus_");
  strcat(outputname, filename);
  printResults(outputname, sets);
  if(dolog)
  fclose(logfile);
  
//generateAllConsensusGroups(sets);
  //  printWeightedResultsGamsFile(filename);  

}



void generateAllConsensusGroups(list * groups)
{
  node * group1;
  node * group2;
  //  node * opt1, *opt2;

  list newbies;

  allconsensusgroups = (struct hashtable *) create_hashtable(groups->count * groups->count, 
							     getAssignmentBasedGroupHash, equalsAssignmentGroups);

    for(group1 = groups->head; group1 != NULL; group1 = group1->next)
      {
	sibgroup * gg = group1->data;
	if(gg->founder.y == 5376)
	  printf(" ");
	//      hashtable_insert(allconsensusgroups, group1, group1);
      }

  //  group1 = groups->head;
  copyList(& newbies, groups);

  double optcost = 9999999;
  int merging = 0;
  do{
    list deletelist;
    deletelist.head = NULL;
    deletelist.count = 0;


    merging = 0;
    list newnewbies;
    newnewbies.head = NULL;
    newnewbies.count = 0;
    //x    opt2 = opt1 = NULL;
    optcost = 9999999;
    group1 = groups->head;
    printf("Newbies at start of loop: %d \n", newbies.count);

    printWeightedResultsGamsFile("loop_groups");
    struct hashtable * newbiestable = (struct hashtable *) create_hashtable(data.count * data.count, 
							           getAssignmentBasedGroupHash, equalsAssignmentGroups);

    for(group1 = newbies.head ; group1!= NULL; group1 = group1->next)
      {
	/** @todo: compare edit here for newbies **/
	if(hashtable_search(allconsensusgroups, group1->data) == NULL
	   && hashtable_search(newbiestable, group1->data) == NULL)
	  hashtable_insert(newbiestable, group1->data, group1->data);
      }
    struct hashtable_iterator * itr1, * itr2;
    //    itr1 = hashtable_iterator(allconsensusgroups);
    //itr2 = hashtable_iterator(allconsensusgroups);
    int c1 = 0;
    int c2 = 0;
    int totalinds = 0;
    for(group1 = newbies.head ; group1!= NULL; group1 = group1->next)
      {
	sibgroup * gp1 = (sibgroup*) group1->data;
	c1++;
	c2 = c1;
	for(group2 = group1->next; group2 != NULL; group2 = group2->next )
	  {
	    c2++;
	    optcost = 9999;
	    sibgroup * gp2 = (sibgroup*) group2->data;
	    sibgroup * gp, *ngp;
	    double mcost = getMergeCost(gp1, gp2);
	    double mcost2 = getMergeCost(gp2, gp1);
	    totalinds = gp1->assignments->count + gp2->assignments->count;
	    if(mcost < optcost)
	      {
		//		opt1 = group1;
		//opt2 = group2;
		optcost = max(0, mcost) + gp1->edit;
		gp = gp1;
		ngp = gp2;
		
	      }
	    if (mcost2 < optcost)
	      {
		
		//		opt1 = group1;
		//opt2 = group2;
		optcost = max(0, mcost2) + gp2->edit;
		gp = gp2;
		ngp = gp1;
	      }
	    //	    printf("Gp:%d + Gp %d = %2.3f\n", c1, c2, optcost);
	    if(optcost <= costs[MAX_GROUP_EDIT] && optcost <= ceil(costs[MAX_ERROR_RATE]*totalinds))
	      {
		sibgroup * merge = copySibgroup(gp);
		merge->edit = optcost;
		node * iptr = data.head;
		int i= 0;
		node * ptr;
		for(ptr = ngp->assignments->head ; ptr != NULL ; ptr = ptr->next)
		  {
	    
		    if(optcost < 0)
		      {
			for(; iptr != NULL && i != ptr->data; iptr= iptr->next, i ++);
			assignIndividualToGroup(merge, iptr->data);
		      }
		    addToSortedList(merge->assignments, ptr->data);
		  }
		sibgroup * g = hashtable_search(newbiestable, merge);
		if(g!= NULL && g->edit > merge->edit)
		  {
		    sibgroup * g2; // we don't need something we already have in allconsensusgroups 
		  if( (NULL == (g2 = hashtable_search(allconsensusgroups, merge))) ||
		      g2->edit > merge->edit)
		    {
		      deleteItem(& newbies, g, blank);
		      hashtable_remove_clear(newbiestable, g, 0);
		      sibgroup * gp = malloc(sizeof(sibgroup));
		      *gp = *g;
		      addToList(& deletelist, gp);
		      //	      deleteSibgroupShallow(g, 1);
		      hashtable_insert(newbiestable, merge, merge);
		      addToList(& newbies, merge);
		    }
		  else{
		    deleteSibgroup(merge);
		  }
		}else if(g == NULL){

		  sibgroup * g;
		  if( (NULL == (g = hashtable_search(allconsensusgroups, merge))) ||
		      g->edit > merge->edit)
		    {

		      /*		      if(hashtable_count(merge->intmaps[0]) == 0)
					      printf("ERROR: empty allele maps");*/
		      addToList(&newnewbies, merge);		      
		    
		      //		      hashtable_insert(newbiestable, merge, merge);
		    }
		}else
		  {
		    deleteSibgroup(merge);
		  }


		merging = 1;
	      }
	    
	  }
      }
    // now let's compare newbies to what we have
    itr1  = hashtable_iterator(allconsensusgroups);
    newbies = newnewbies;
    /** todo: stop mem leaks */

    //for(group1 = newbies.head ; group1!= NULL; group1 = group1->next)
    do {
      if(hashtable_count(allconsensusgroups) <= 0) continue;
      itr2 = hashtable_iterator(newbiestable);
      sibgroup * gp1 = hashtable_iterator_key(itr1);
//	for(group2 = group1->next; group2 != NULL; group2 = group2->next )
      do  {
	if(hashtable_count(newbiestable) == 0)
	  continue;
	sibgroup * gp2 = hashtable_iterator_key(itr2);
	optcost = 9999;
	sibgroup * gp, *ngp;
	    double mcost = getMergeCost(gp1, gp2);
	    double mcost2 = getMergeCost(gp2, gp1);
	    totalinds = gp1->assignments->count + gp2->assignments->count;
	    if(mcost < optcost)
	      {
		//		opt1 = group1;
		//opt2 = group2;
		optcost = max(0, mcost) + gp1->edit;
		gp = gp1;
		ngp = gp2;
	      }
	    if (mcost2 < optcost)
	      {
		
		//opt1 = group1;
		//opt2 = group2;
		optcost = max(0, mcost2 )+ gp2->edit;
		gp = gp2;
		ngp = gp1;
	      }

	    if(optcost <= costs[MAX_GROUP_EDIT] && optcost <= ceil(costs[MAX_ERROR_RATE] * totalinds))
	      {
		sibgroup * merge = copySibgroup(gp);
		node * iptr = data.head;
		merge->edit = optcost;
		int i= 0;
		node * ptr;
		for(ptr = ngp->assignments->head ; ptr != NULL ; ptr = ptr->next)
		  {
		    if(optcost < 0)
		      {
			for(; iptr != NULL && i != ptr->data; iptr= iptr->next, i ++);
			assignIndividualToGroup(merge, iptr->data);
		      }
		    addToSortedList(merge->assignments, ptr->data);
		  }

		sibgroup * g = hashtable_search(newbiestable, merge);


		if(g!= NULL && g->edit > merge->edit)
		  {
		    sibgroup * g2; // we don't need something we already have in allconsensusgroups 
		    if( (NULL == (g2 = hashtable_search(allconsensusgroups, merge))) )
		    {
		      //  hashtable_remove(newbiestable, g);
		      //	      deleteSibgroupShallow(g, 1);
		      // hashtable_insert(newbiestable, merge, merge);
		      addToList(&newbies, merge);
		      if( hashtable_count( merge->intmaps[0]) == 0)
			printf("ERROR");
		    }else if(g2->edit > merge->edit)
		      {
			hashtable_remove_clear(newbiestable, g, 0);
			hashtable_insert(newbiestable, merge, merge);
		      }
		    else{
		      deleteSibgroup(merge);
		    }
		}else if (g == NULL){

		  sibgroup * g;
		  if( (NULL == (g = hashtable_search(allconsensusgroups, merge))) ||
		      g->edit > merge->edit)
		    {		    
		      addToList(&newbies, merge);
		      if(hashtable_count(merge->intmaps[0]) == 0)
			printf("ERROR");
		      //hashtable_insert(newbiestable, merge, merge);
		    }
		  else{
		    deleteSibgroup(merge);
		  }
		}else
		  {
		    deleteSibgroup(merge);
		  }

		merging = 1;
	      }
	    
      }while(hashtable_iterator_advance(itr2));
      free(itr2);
    }while(hashtable_iterator_advance(itr1));
    free(itr1);

    // now finally dump everything in newbiesmap into allconsensusgroups
    itr1 = hashtable_iterator(newbiestable);

    do{
      sibgroup * gp1 = hashtable_iterator_key(itr1);
      sibgroup * gp2 = hashtable_search(allconsensusgroups, gp1);
      if(gp2 == NULL || gp2->edit > gp1->edit)
	{
	  if(gp2 != NULL) // must remove earlier
	    {
	    }
	  sibgroup * gp = malloc(sizeof(sibgroup));
	  *gp = *gp1;
	  hashtable_insert(allconsensusgroups, gp, gp);
	}
      else
	{
	  sibgroup * gp = malloc(sizeof(sibgroup));
	  *gp = *gp1;
	  addToList(&deletelist, gp);
	}
    }while(hashtable_iterator_advance(itr1));
    /** @todo: still a mem leak **/
    free(itr1);

    hashtable_destroy(newbiestable, 0);
    deleteList(& deletelist, deleteSibgroup);
    //    free(newbiestable);
    //    newbiestable = (struct hashtable *) create_hashtable(newbies.count * newbies.count, 
    //			           getAssignmentBasedGroupHash, equalsAssignmentGroups);		
  }while( merging);

  
}

void dropLocus(int locusdrop, list sourcedata)
{
  /*  if(lociorder == NULL)
    {
      lociorder = calloc(nloci, sizeof(int));
    }
  int i ,cl, j;
  for(i =0, cl =0; i < nloci+1; i ++)
    {
      if(i != locusdrop)
	lociorder[cl++]=i;
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
	  }*/
  node * current = sourcedata.head;
  data.head =NULL;
  data.count =0;
  int cl,i;
  for( ; current != NULL; current = current->next)
    {
      dataentry * e = current->data;
      Pair * loci = calloc(nloci, sizeof(Pair));
      dataentry *ne = malloc(sizeof(dataentry));
      //      *ne = *e;
      strcpy(ne->name, e->name);
      ne->index = e->index;
      ne->loci = loci;
      for(cl = 0, i = 0; i < nloci+1; i ++)
	{
	  if(locusdrop != i)
	    loci[cl++]=e->loci[i];
	}
      addToList(&data, ne);
      
    }
        sortLociByAlleleFrequency();
	//  sortByAlleleFrequency();


}

int getAssignmentBasedGroupHash(sibgroup * group)
{
  if(hashtable_count(group->intmaps[0]) == 0)
    printf("ERROR: empty allele maps");
  int hash = 0;
  node * ptr = group->assignments->head;

  for( ; ptr != NULL; ptr = ptr->next)
    {
      hash ^= ( (((int)ptr->data )<< (((int)ptr->data) % 8)) ^ (((int)ptr->data) >> (((int)ptr->data) %8)));
    }
  return hash;
}


int equalsAssignmentGroups(sibgroup * group1, sibgroup * group2)
{

  if(hashtable_count(group1->intmaps[0]) == 0)
    printf("ERROR: empty allele maps");
 

  if(hashtable_count(group2->intmaps[0]) == 0)
    printf("ERROR: empty allele maps");

  node * p1, * p2;
  p1 = group1->assignments->head;
  p2 = group2->assignments->head;
  while( p1 != NULL && p2 != NULL)
    {
      if(p1->data != p2->data)
	return 0;
      p1 = p1->next;
      p2 = p2->next;
    }
  if(p1 == NULL && p2 == NULL)
    return 1;
  else
    return 0;
}
