
# INTV = -DLONG
INTE = -DEDGELONG

#compilers
$(info ************  Using CILK ************)
PCC = g++-7
PCFLAGS = -std=c++14 -fcilkplus -lcilkrts -g -O3 -DCILK $(INTV) $(INTE) -DNoPageRank

# dependencies
COMMON = ../../core/common/utils.h ../../core/common/parseCommandLine.h ../../core/common/parallel.h ../../core/common/quickSort.h ../../core/common/blockRadixSort.h ../../core/common/transpose.h ../../core/common/rwlock.h
LOCAL_COMMON = ../common/graphIO.h ../common/fileUtils.h

CONVERTERS = updateGenerator

.PHONY: all clean run

all: $(CONVERTERS)

% : %.C $(COMMON) $(LOCAL_COMMON)
	$(PCC) $(PCFLAGS) -o $@ $<

clean :
	rm -f *.o $(CONVERTERS)

run : all
	./updateGenerator ../../inputs/$(FILE_NAME)_seed_file ../../inputs/$(FILE_NAME).snap ../../inputs/$(FILE_NAME)_init.snap ../../inputs/$(FILE_NAME)_operations.txt $(BASE_GRAPH_RATE) $(BATCH_ADD_RATE) $(BATCH_SIZE) $(BATCH_TIME) $(OUTPUT_STD)