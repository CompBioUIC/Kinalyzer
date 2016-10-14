# Makefile - PKin

CXX=g++
IXX=icpc
CXXFLAGS=-I ../../external/boost_1_42_0 -fpic -W -DUNIX=1
LD=$(CXX) $(CXXFLAGS)
ILD=$(IXX) $(CXXFLAGS)
LIBS+=-lstdc++ -lm -lpthread
OUTPUT_DIR=../../../binaries/PKin/

FLAGS= -O3 -DPk_ENABLE_ASSERT=0 -DPk_ENABLE_LOGGING=1
MPI_TARGET=$(OUTPUT_DIR)PKin
LOCAL_TARGET=$(OUTPUT_DIR)PKin-local

DEBUG_FLAGS= -O0 -fexceptions -g  -fno-inline -DPk_ENABLE_ASSERT=1 -DPk_ENABLE_LOGGING=1
MPI_DEBUG_TARGET=$(OUTPUT_DIR)PKin-debug
LOCAL_DEBUG_TARGET=$(OUTPUT_DIR)PKin-debug-local

OPT_DEBUG_FLAGS= -O3 -DPk_ENABLE_ASSERT=1 -DPk_ENABLE_LOGGING=1
MPI_OPT_DEBUG_TARGET=$(OUTPUT_DIR)PKin-optdebug
LOCAL_OPT_DEBUG_TARGET=$(OUTPUT_DIR)PKin-optdebug-local

MPI_ARGO_FLAGS= -I /usr/common/mpich2-1.0.7/include -DMPICH_IGNORE_CXX_SEEK -DARGO=1
MPI_ARGO_LIB_FLAGS= -L /usr/common/mpich2-1.0.7/lib
MPI_ARGO_LIBS= -lmpich

SOURCE_FILES= \
	./PkAssert.cpp \
	./PkBitSetUtils.cpp \
	./PkClock.cpp \
	./PkCommandletThreadPoolTest.cpp \
	./PkConsensusCosts.cpp \
	./PkConsensusExecGamsAndAllScripts.cpp \
	./PkConsensusGreedyMergeGroups.cpp \
	./PkConsensusLociIntersectionAndOutputResults.cpp \
	./PkConsensusMain.cpp \
	./PkConsensusOutputSolution.cpp \
	./PkConsensusSetCoverParser.cpp \
	./PkConsensusStrictVote.cpp \
	./PkFileUtils.cpp \
	./PkFileUtilsLinux.cpp \
	./PkFileUtilsWindows.cpp \
	./PkGamsUtils.cpp \
	./PkGlobalData.cpp \
	./PkLocalLociReconstructionClusterCentralized.cpp \
	./PkLocalLociReconstructionClusterIsland.cpp \
	./PkLocalLociReconstructionClusterWorkStealing.cpp \
	./PkLocalSiblingGroupReconstructor.cpp \
	./PkMain.cpp \
	./PkParallelIntersectorUtil.cpp \
	./PkParentRemapper.cpp \
	./PkPhaseLocalLociReconstruction.cpp \
	./PkPhaseLociIntersection.cpp \
	./PkPhaseOutputResults.cpp \
	./PkPlatformThreadPolicyBoost.cpp \
	./PkPlatformThreadPolicyUnix.cpp \
	./PkPlatformThreadPolicyWindows.cpp \
	./PkPoolThread.cpp \
	./PkPopulationLoader.cpp \
	./PkPossibleParentsMap.cpp \
	./PkSiblingGroup.cpp \
	./PkSiblingSetIntersector.cpp \
	./PkStats.cpp \
	./PkThreadPool.cpp \
	./csvparser.cpp \

HEADER_FILES= \
	./PkAppHooks.h \
	./PkAssert.h \
	./PkBitSetUtils.h \
	./PkBlockableRunnable.h \
	./PkBuild.h \
	./PkClock.h \
	./PkCommandletThreadPoolTest.h \
	./PkConsensus.h \
	./PkConsensusCosts.h \
	./PkConsensusSetCoverParser.h \
	./PkConsensusSiblingGroup.h \
	./PkCriticalSection.h \
	./PkCriticalSectionBoost.h \
	./PkCriticalSectionUnix.h \
	./PkCriticalSectionWindows.h \
	./PkEvent.h \
	./PkEventBoost.h \
	./PkEventUnix.h \
	./PkEventWindows.h \
	./PkFileUtils.h \
	./PkFileUtilsLinux.h \
	./PkFileUtilsWindows.h \
	./PkFixedSizeArray.h \
	./PkFixedSizeStack.h \
	./PkGamsUtils.h \
	./PkGlobalData.h \
	./PkIndividual.h \
	./PkLocalLociReconstructionClusterCentralized.h \
	./PkLocalLociReconstructionClusterInterface.h \
	./PkLocalLociReconstructionClusterIsland.h \
	./PkLocalLociReconstructionClusterWorkStealing.h \
	./PkLocalSiblingGroupReconstructor.h \
	./PkLociArray.h \
	./PkLocus.h \
	./PkMiscUtil.h \
	./PkParallelIntersectorUtil.h \
	./PkParentRemapper.h \
	./PkParentSet.h \
	./PkPhases.h \
	./PkPlatformThreadPolicy.h \
	./PkPlatformThreadPolicyBoost.h \
	./PkPlatformThreadPolicyUnix.h \
	./PkPlatformThreadPolicyWindows.h \
	./PkPoolThread.h \
	./PkPopulationLoader.h \
	./PkPossibleParentsMap.h \
	./PkRand.h \
	./PkRunnable.h \
	./PkScopeLock.h \
	./PkSiblingGroup.h \
	./PkSiblingGroupCommandBufferCentralized.h \
	./PkSiblingGroupCommandBufferInterface.h \
	./PkSiblingGroupCommandBufferIsland.h \
	./PkSiblingGroupCommandBufferWorkStealing.h \
	./PkSiblingGroupUnorderedSet.h \
	./PkSiblingSetIntersector.h \
	./PkStats.h \
	./PkStatsDetailedSubsetCullingStatTracker.h \
	./PkSubsetCuller.h \
	./PkSubsetCullingParallelPolicy.h \
	./PkSubsetCullingSerialPolicy.h \
	./PkSubsetFrequencyMapper.h \
	./PkSubsetOracle.h \
	./PkThreadPool.h \
	./PkTypeTraits.h \
	./PkTypes.h \
	./csvparser.h

SRCS=$(SOURCE_FILES) $(HEADER_FILES) 

OBJS=$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS))))))

BINS=$(LOCAL_TARGET) $(LOCAL_DEBUG_TARGET) $(LOCAL_OPT_DEBUG_TARGET) $(MPI_TARGET) $(MPI_DEBUG_TARGET) $(MPI_OPT_DEBUG_TARGET) $(LOCAL_TARGET)i $(LOCAL_DEBUG_TARGET)i $(LOCAL_OPT_DEBUG_TARGET)i $(MPI_TARGET)i $(MPI_DEBUG_TARGET)i $(MPI_OPT_DEBUG_TARGET)i

# target used for compiling local implementation of PKin for any Unix machine
all: $(SRCS)
	$(CXX) $(CXXFLAGS) $(FLAGS) -c $(SOURCE_FILES)
	$(LD) -o $(LOCAL_TARGET) $(OBJS) $(LIBS)

# target used for compiling debug local implementation of PKin for any Unix machine
debug: $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $(SOURCE_FILES)
	$(LD) -o $(LOCAL_DEBUG_TARGET) $(OBJS) $(LIBS)

# target used for compiling optimized debug local implementation of PKin for any Unix machine
optdebug: $(SRCS)
	$(CXX) $(CXXFLAGS) $(OPT_DEBUG_FLAGS) -c $(SOURCE_FILES)
	$(LD) -o $(LOCAL_OPT_DEBUG_TARGET) $(OBJS) $(LIBS)

# target used for compiling local implementation of PKin on the intel compiler
intel: $(SRCS)
	$(IXX) -fast $(CXXFLAGS) $(FLAGS) -c $(SOURCE_FILES)
	$(ILD) -o $(LOCAL_TARGET)i $(OBJS) $(LIBS)

# target used for compiling optimized debug local implementation of PKin on the intel compiler
inteloptdebug: $(SRCS)
	$(IXX) -fast $(CXXFLAGS) $(OPT_DEBUG_FLAGS) -c $(SOURCE_FILES)
	$(ILD) -o $(LOCAL_OPT_DEBUG_TARGET)i $(OBJS) $(LIBS)

# target used for compiling MPI distributed implementation of PKin for ARGO cluster
argo: $(SRCS)
	$(CXX) $(CXXFLAGS) $(FLAGS) $(MPI_ARGO_FLAGS) -c $(SOURCE_FILES)
	$(LD) $(MPI_ARGO_LIB_FLAGS) -o $(MPI_TARGET) $(OBJS) $(LIBS) $(MPI_ARGO_LIBS)

# target used for compiling debug MPI distributed implementation of PKin for ARGO cluster
argodebug: $(SRCS)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(MPI_ARGO_FLAGS) -c $(SOURCE_FILES)
	$(LD) $(MPI_ARGO_LIB_FLAGS) -o $(MPI_DEBUG_TARGET) $(OBJS) $(LIBS) $(MPI_ARGO_LIBS)

# target used for compiling optimized debug MPI distributed implementation of PKin for ARGO cluster
argooptdebug: $(SRCS)
	$(CXX) $(CXXFLAGS) $(OPT_DEBUG_FLAGS) $(MPI_ARGO_FLAGS) -c $(SOURCE_FILES)
	$(LD) $(MPI_ARGO_LIB_FLAGS) -o $(MPI_OPT_DEBUG_TARGET) $(OBJS) $(LIBS) $(MPI_ARGO_LIBS)

# target used for cleaning all object and binary files
clean:
	-rm -f -v *.o $(BINS)

# target used for cleaning all local object files
cleanobjs:
	-rm -f -v *.o
