PWD             = $(shell pwd)
FILE_NAME       = Wiki-Vote
BATCH_SIZE      = 100
BATCH_TIME      = 1
SNAP_VERTEX_NUM = 524288
SNAP_EDGE_NUM   = 7340032
BASE_GRAPH_RATE = 0.5
BATCH_ADD_RATE  = 0.7
OUTPUT_STD      = ~/tmp/output_std/pr_output
OUTPUT          = ~/tmp/output1/pr_output
DIFF            = ~/tmp/diff/pr_output
CORE_NUM        = 52
DEGREE_AVG      = 14
RF_OUTPUT       = -graphboltIters 5 -tegraIters 5

tools  = $(PWD)/tools
inputs = $(PWD)/inputs
apps   = $(PWD)/apps

.PHONY: Snap2Adj Generator PageRank PRCompare RunAll DEL_NOTES_TXT PageRankRuns ANALYSIS RF_TRAIN RF_PREDICT PageRankDelta PageRankTrad PageRankAe PageRankTegra PageRankMechine AutoRun MechineRun

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF CORE_NUM SNAP_VERTEX_NUM SNAP_EDGE_NUM BASE_GRAPH_RATE BATCH_ADD_RATE DEGREE_AVG RF_OUTPUT

Generator :
	$(MAKE) -C $(tools)/updateGenerator run

Snap2Adj :
	$(MAKE) -C $(tools)/converters run

PageRank :
	$(MAKE) -C $(apps) PageRankRun

PRCompare :
	$(MAKE) -C $(tools)/output_comparators PRCompare

PageRankDelta :
	$(MAKE) -C $(apps) PageRankRunDelta

PageRankTrad :
	$(MAKE) -C $(apps) PageRankRunTrad OUTPUT=$(OUTPUT_STD)

PageRankAe :
	$(MAKE) -C $(apps) PageRankRunAe

PageRankTegra:
	$(MAKE) -C $(apps) PageRankRunTegra

PageRankMechine:
	$(MAKE) -C $(apps) PageRankRunMechine

PageRankRuns :
	$(MAKE) -C $(apps) PageRankRuns

LpTrad :
	$(MAKE) -C $(apps) LpRunTrad OUTPUT=$(OUTPUT_STD)

LpDelta :
	$(MAKE) -C $(apps) LpRunDelta

LpTegra :
	$(MAKE) -C $(apps) LpRunTegra

CoemTrad :
	$(MAKE) -C $(apps) CoemRunTrad OUTPUT=$(OUTPUT_STD)

CoemDelta :
	$(MAKE) -C $(apps) CoemRunDelta

CoemTegra :
	$(MAKE) -C $(apps) CoemRunTegra

RMAT_Generator:
	$(MAKE) -C $(tools)/PaRMAT/Release RMAT_Generator

ANALYSIS:
	$(MAKE) -C $(tools)/analysis run

RF_TRAIN:
	$(MAKE) -C $(tools)/mechine_rf TRAIN
	
RF_PREDICT:
	$(MAKE) -C $(tools)/mechine_rf PREDICT
	$(eval RF_OUTPUT := $(shell python3 ./tools/mechine_rf/predict.py $(BATCH_SIZE) $(SNAP_VERTEX_NUM) $(SNAP_EDGE_NUM) $(BATCH_ADD_RATE) $(DEGREE_AVG)))
	@echo "RF_OUTPUT = "  $(RF_OUTPUT)

RF_EVAL:
	$(MAKE) -C $(tools)/mechine_rf EVAL

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

AutoRun: Generator Snap2Adj PageRankDelta PageRankTrad PageRankTegra
	echo "AutoRun finish"

RunPy:
	for i in {1..100}; do \
		python3 run.py; \
	done

MechineRun: Generator Snap2Adj RF_PREDICT
	make PageRankMechine RF_OUTPUT="$(RF_OUTPUT)"
	make PageRankMechine RF_OUTPUT="$(RF_OUTPUT)"
	make PageRankMechine RF_OUTPUT="$(RF_OUTPUT)"
	make PageRankDelta
	make PageRankDelta
	make PageRankDelta
	make PageRankTrad
	make PageRankTrad
	make PageRankTrad
	make PageRankTegra
	make PageRankTegra
	make PageRankTegra
	echo "MechineRunOver"