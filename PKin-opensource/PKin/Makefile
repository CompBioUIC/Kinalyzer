# Makefile - PKin

# target used for compiling local implementation of PKin for any Unix machine
all:
	@echo Compiling RELEASE build
	cd ./ && $(MAKE) -f PKin.mak
# target used for compiling debug local implementation of PKin for any Unix machine
debug:
	@echo Compiling DEBUG build
	cd ./ && $(MAKE) -f PKin.mak debug

# target used for compiling optimized debug local implementation of PKin for any Unix machine
optdebug:
	@echo Compiling OPTDEBUG build
	cd ./ && $(MAKE) -f PKin.mak optdebug

# target used for compiling local implementation of PKin on the intel compiler
intel:
	@echo Compiling INTEL RELEASE build
	cd ./ && $(MAKE) -f PKin.mak intel

# target used for compiling optimized debug local implementation of PKin on the intel compiler
inteloptdebug:
	@echo Compiling INTEL OPTDEBUG build
	cd ./ && $(MAKE) -f PKin.mak inteloptdebug

# target used for compiling MPI distributed implementation of PKin for ARGO cluster
argo:
	@echo Compiling MPI RELEASE build
	cd ./ && $(MAKE) -f PKin.mak argo

# target used for compiling debug MPI distributed implementation of PKin for ARGO cluster
argodebug:
	@echo Compiling MPI DEBUG build
	cd ./ && $(MAKE) -f PKin.mak argodebug

# target used for compiling optimized debug MPI distributed implementation of PKin for ARGO cluster
argooptdebug:
	@echo Compiling MPI OPTDEBUG build
	cd ./ && $(MAKE) -f PKin.mak argooptdebug

# target used for cleaning all object and binary files
clean:
	cd ./ && $(MAKE) -f PKin.mak clean

# target used for cleaning local object files
cleanobjs:
	cd ./ && $(MAKE) -f PKin.mak cleanobjs
