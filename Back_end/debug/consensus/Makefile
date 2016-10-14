
default: gccdebug
	

icc: main.h structures.h main.c utilities.c hashtable.c
	icc -fast main.c utilities.c consensus.c hashtable.c hashtable_itr.c -o isets

prime: main.h structures.h main.c utilities.c hashtable_prime.c
	icc -fast -lm main.c utilities.c hashtable_prime.c hashtable_itr.c -o isets
gcc:
	cc -O -lm consensus.c main.c utilities.c hashtable.c hashtable_itr.c -o sets
gccdebug:
	cc -g -lm consensus.c main.c utilities.c hashtable.c hashtable_itr.c -o setsdebug



debug: default
	icc -g -lm main.c utilities.c hashtable.c hashtable_itr.c consensus.c -o isetsdebug
