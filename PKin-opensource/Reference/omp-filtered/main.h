/***********************************************************
 * main.h
 * author: saad sheikh
 * Date Created: July 23, 2007
 *
 * Contains common variables and non-structural definitions
 **********************************************************/


#ifndef MAIN_H
#define MAIN_H


#define NULL_ALLELE_ERROR 0
#define HOMO_MISTYPE_ERROR 1
#define HETERO_MISTYPE_ERROR 2
#define POSSIBILITY_MISFIT 3
#define MAX_GROUP_EDIT 4
#define MAX_ERROR_RATE 5
#define errors stderr
#define INFINITY 9999

// Global Variables
//itemsetsmap
extern list data;
extern struct hashtable * possibilitiesMap;
extern int nloci;
extern int dolog;
extern int individuals;
extern int mothersmode;
extern individual * mother;
extern int * allassignments;
extern double * costs;
extern double maxedit;
extern struct hashtable ** alleles;
extern const int MAX_POSSIBILITIES;
extern const int ABSENT_ALLELE ;
extern const int MAX_ALLELES_PER_LOCUS;
//extern struct  hashtable * sibgroupsmap;
extern list * possibilities;
//extern int * lociorder;
extern FILE * dumpfile;
extern FILE * logfile;
// 
extern int maxthreads;
extern int ** motherpossibsmap;

void readData(const char* );
void doConsensus(char * datafile, char ** setfiles, int count);
void doGreedyConsensus(char * datafile, char ** setfiles, int count);

#endif
