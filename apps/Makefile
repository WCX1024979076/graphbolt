# Enable this if the input graph has weights. Disable this if you want to use synthetically generated weights.
ifdef WEIGHTED
EDGEDATA = -DEDGEDATA
endif

# Enable this if number of vertices > 2^32 
ifdef LONGVERTEXCOUNT
INTV = -DLONG
endif

INTE = -DEDGELONG

CORE_LIST := $(shell seq -s, 26 $$(expr 25 + $(CORE_NUM) / 2)),$(shell seq -s, 78 $$(expr 77 + $(CORE_NUM) / 2))

#compilers
$(info ************  Using CILK ************)
PCC = g++-7
PCFLAGS = -std=c++14 -fcilkplus -lcilkrts -g -O3 -DCILK $(INTV) $(INTE) $(EDGEDATA)
LDFLAGS = -L../lib/mimalloc/out/release -lmimalloc 

# dependencies
COMMON=../core/common/binary_search.h ../core/common/bitsetscheduler.h ../core/common/blockRadixSort.h ../core/common/densebitset.h ../core/common/gettime.h ../core/common/index_map.h ../core/common/matrix.h ../core/common/maybe.h ../core/common/parallel.h ../core/common/parseCommandLine.h ../core/common/transpose.h ../core/common/quickSort.h ../core/common/rwlock.h ../core/common/sequence.h ../core/common/utils.h

GRAPH= ../core/graph/graph.h ../core/graph/graphUtils.h ../core/graph/IO.h ../core/graph/vertex.h ../core/graph/vertexSubset.h

GRAPHBOLT=../core/graphBolt/AdaptiveExecutor.h ../core/graphBolt/GraphBoltEngine_complex.h ../core/graphBolt/GraphBoltEngine.h ../core/graphBolt/GraphBoltEngine_simple.h ../core/graphBolt/ingestor.h ../core/graphBolt/KickStarterEngine.h

OTHERS=../core/main.h
	
ALL=PageRank LabelPropagation CF COEM SSSP BFS

# make

all: $(ALL)

% : %.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) $< $(LDFLAGS) -o $@

LpTrad : LabelPropagation.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) $< $(LDFLAGS) -o $@

LpDelta : LabelPropagation.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -Ddelta_calc $< $(LDFLAGS) -o $@

LpTegra : LabelPropagation.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -Dtegra_calc $< $(LDFLAGS) -o $@

LpRunTrad : LpTrad
	taskset -c $(CORE_LIST) ./LpTrad -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT)  -seedsFile ../inputs/$(FILE_NAME)_seed_file ../inputs/$(FILE_NAME)_init.adj 

LpRunDelta : LpDelta
	taskset -c $(CORE_LIST) ./LpDelta -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT)  -seedsFile ../inputs/$(FILE_NAME)_seed_file ../inputs/$(FILE_NAME)_init.adj 

LpRunTegra : LpTegra
	taskset -c $(CORE_LIST) ./LpTegra -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT)  -seedsFile ../inputs/$(FILE_NAME)_seed_file ../inputs/$(FILE_NAME)_init.adj 

PageRankDelta : PageRank.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -Ddelta_calc $< $(LDFLAGS) -o $@

PageRankTrad : PageRank.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) $< $(LDFLAGS) -o $@

PageRankAe : PageRank.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -Ddelta_calc $< $(LDFLAGS) -o $@

PageRankTegra : PageRank.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -Dtegra_calc $< $(LDFLAGS) -o $@

PageRankMechine : PageRank.C $(COMMON) $(GRAPH) $(GRAPHBOLT) $(OTHERS)
	$(PCC) $(PCFLAGS) -DMECHINE_ITER $< $(LDFLAGS) -o $@

PageRankRunDelta: PageRankDelta
	taskset -c $(CORE_LIST) ./PageRankDelta -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj

PageRankRunTrad: PageRankTrad
	taskset -c $(CORE_LIST) ./PageRankTrad -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj

PageRankRunAe: PageRankAe
	taskset -c $(CORE_LIST) ./PageRankAe -nWorkers $(CORE_NUM) -ae true -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj

PageRankRunTegra: PageRankTegra
	taskset -c $(CORE_LIST) ./PageRankTegra -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj

PageRankRunMechine: PageRankMechine
	taskset -c $(CORE_LIST) ./PageRankMechine -nWorkers $(CORE_NUM) -ae true -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) -graphboltIters $(GRAPHBOLT_ITER) ../inputs/$(FILE_NAME)_init.adj

.PHONY : clean PageRankRunDelta PageRankRun PageRankRuns PageRankRunDelta PageRankRunTrad PageRankRunAe PageRankRunMechine PageRankRunTegra

clean :
	rm -f *.o *.obj $(ALL)

PageRankRun : PageRank
	taskset -c $(CORE_LIST) ./PageRank -nWorkers $(CORE_NUM) -ae true -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj

PageRankRuns : PageRankDelta PageRankAe PageRankTrad PageRankTegra
	for i in {3..5}; do \
	taskset -c $(CORE_LIST) ./PageRankDelta -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj; \
	taskset -c $(CORE_LIST) ./PageRankTegra -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj; \
	taskset -c $(CORE_LIST) ./PageRankTrad  -nWorkers $(CORE_NUM) -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ../inputs/$(FILE_NAME)_operations.txt -outputFile $(OUTPUT) ../inputs/$(FILE_NAME)_init.adj; \
	done

	echo "finish"
