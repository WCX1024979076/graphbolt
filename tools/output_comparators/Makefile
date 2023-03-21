#INTT = -DLONG
#INTE = -DEDGELONG

#compilers
$(info ************  Using CILK ************)
PCC = g++-7
PCFLAGS = -std=c++14 -fcilkplus -lcilkrts -g -O3 -DCILK $(INTT) $(INTE)

# dependencies
COMMON = ../../core/common/utils.h ../../core/common/parseCommandLine.h ../../core/common/parallel.h ../../core/common/quickSort.h ../../core/common/blockRadixSort.h ../../core/common/transpose.h ../../core/common/rwlock.h
LOCAL_COMMON = ../common/graphIO.h ../common/fileUtils.h

OUTPUT_COMPARATORS = CFOutputComparator PROutputComparator LPOutputComparator CoemOutputComparator

.PHONY: all clean

all: $(OUTPUT_COMPARATORS)

% : %.C $(COMMON) $(LOCAL_COMMON)
	$(PCC) $(PCFLAGS) -o $@ $<

clean :
	rm -f *.o $(OUTPUT_COMPARATORS)

PRCompare : PROutputComparator
	./PROutputComparator -file1 /tmp/output/pr_output0 -file2 /tmp/output_std/pr_output0 -ofile ./run.log