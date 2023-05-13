PWD = $(shell pwd)
FILE_NAME = rmat
BATCH_SIZE = 100000
BATCH_TIME = 10
SNAP_VERTEX_NUM = 750000
SNAP_EDGE_NUM = 4194304
BASE_GRAPH_RATE = 0.5
BATCH_ADD_RATE = 0.7
OUTPUT_STD = ~/tmp/output_std/pr_output
OUTPUT     = ~/tmp/output1/pr_output
DIFF       = ~/tmp/diff/pr_output
CORE_NUM   = 4
DEGREE_AVG = 0.0
GRAPHBOLT_ITER = 0

tools = $(PWD)/tools
inputs = $(PWD)/inputs
apps = $(PWD)/apps

.PHONY: Snap2Adj Generator PageRank PRCompare RunAll DEL_NOTES_TXT PageRankRuns ANALYSIS RF_TRAIN RF_PREDICT

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF CORE_NUM SNAP_VERTEX_NUM SNAP_EDGE_NUM BASE_GRAPH_RATE BATCH_ADD_RATE DEGREE_AVG

Generator :
	# 由于子Makefile 不太好传递变量到父 Makefile当中，所以只能这么实现
	$(MAKE) -C $(tools)/updateGenerator run
	$(eval DEGREE_AVG := $(shell ./tools/updateGenerator/updateGenerator ./inputs/$(FILE_NAME).snap ./inputs/$(FILE_NAME)_init.snap ./inputs/$(FILE_NAME)_operations.txt $(BASE_GRAPH_RATE) $(BATCH_ADD_RATE) $(BATCH_SIZE) $(BATCH_TIME) $(OUTPUT_STD)))

Snap2Adj :
	$(MAKE) -C $(tools)/converters run

PageRank :
	$(MAKE) -C $(apps) PageRankRun

PRCompare :
	$(MAKE) -C $(tools)/output_comparators PRCompare

PageRankRuns :
	$(MAKE) -C $(apps) PageRankRuns
	
RMAT_Generator:
	$(MAKE) -C $(tools)/PaRMAT/Release RMAT_Generator

ANALYSIS:
	$(MAKE) -C $(tools)/analysis run

RF_TRAIN:
	$(MAKE) -C $(tools)/mechine_train TRAIN
	
RF_PREDICT:
	$(MAKE) -C $(tools)/mechine_train PREDICT
	$(eval GRAPHBOLT_ITER := $(shell python3 ./tools/mechine_train/predict.py $(BATCH_SIZE) $(SNAP_VERTEX_NUM) $(SNAP_EDGE_NUM) $(BATCH_ADD_RATE) $(DEGREE_AVG)))

DEL_NOTES_TXT:
	@if test -e /home/wangcx/tmp/notes.txt ; \
    then \
        rm /home/wangcx/tmp/notes.txt ; \
        echo "File deleted." ; \
    else \
        echo "File not found." ; \
    fi

RunAll: RMAT_Generator Generator Snap2Adj PageRankRuns ANALYSIS
	echo "finish"

RunPy:
	for i in {1..10}; do \
		python3 run.py; \
	done