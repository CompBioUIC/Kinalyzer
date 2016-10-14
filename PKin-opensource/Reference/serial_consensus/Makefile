OUTPUT_DIR=../../../binaries/serial_consensus/

default: gccdebug
	
icc: main.h structures.h main.c utilities.c hashtable.c
	icc -fast main.c utilities.c consensus.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)isets

prime: main.h structures.h main.c utilities.c hashtable_prime.c
	icc -fast -lm main.c utilities.c hashtable_prime.c hashtable_itr.c -o $(OUTPUT_DIR)isets
gcc:
	cc -O2 -lm consensus.c main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)sets
icc:
	icc -fast consensus.c main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)isets
gccdebug:
	cc -g3 -lm consensus.c main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)setsdebug

debug: default
	icc -g -lm main.c utilities.c hashtable.c hashtable_itr.c consensus.c -o $(OUTPUT_DIR)isetsdebug
