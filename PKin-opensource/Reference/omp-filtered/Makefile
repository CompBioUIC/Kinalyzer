OUTPUT_DIR=../../../binaries/omp-filtered/

default: iccpfast
	
iccpfast2:
	 icc -fast -openmp  main.c utilities-iterative.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsfast  -sunlimited -std=c99

iccpfast:
	 icc -fast -openmp  main.c utilities2.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsfast  -sunlimited -std=c99
iccdebug:
	 icc -O0 -g3 -openmp  main.c utilities2.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsd  -sunlimited -std=c99
iccdebug2:
	 icc -g3 -openmp  main.c utilities-iterative.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsdebug  -sunlimited -std=c99

icc: main.h structures.h main.c utilities.c hashtable.c
	icc -fast main.c utilities.c consensus.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)isets

prime: main.h structures.h main.c utilities.c hashtable_prime.c
	icc -fast -lm main.c utilities.c hashtable_prime.c hashtable_itr.c -o $(OUTPUT_DIR)isets
gccoptim:
	cc -O2 -lm main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psets
gccdebug:
	cc -g3 -lm main.c utilities2.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)setsdebug
gccdebug2:
	cc -g3 -lm main.c utilities-iterative.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)setsdebug
threads:
	cc -fopenmp -lm main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)setsdebug
threads2:
	cc -fopenmp -O2 -lm main.c utilities2.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsfastgcc
threads2d:
	cc -fopenmp -g3 -lm main.c utilities2.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psetsfastgcc
gcc:
	cc -lm  main.c utilities.c hashtable.c hashtable_itr.c -o $(OUTPUT_DIR)psets

debug: default
	icc -g -lm main.c utilities.c hashtable.c hashtable_itr.c  -o $(OUTPUT_DIR)psetsdebug
