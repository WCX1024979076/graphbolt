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

tools = $(PWD)/tools
inputs = $(PWD)/inputs
apps = $(PWD)/apps

.PHONY: Snap2Adj Generator PageRank PRCompare RunAll DEL_NOTES_TXT PageRankRuns ANALYSIS

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF CORE_NUM SNAP_VERTEX_NUM SNAP_EDGE_NUM BASE_GRAPH_RATE BATCH_ADD_RATE

Generator :
	cd $(tools)/updateGenerator && make run

Snap2Adj :
	cd $(tools)/converters && make run

PageRank :
	cd $(apps) && make PageRankRun

PRCompare :
	cd $(tools)/output_comparators && make PRCompare

PageRankRuns :
	cd $(apps) && make PageRankRuns

RMAT_Generator:
	cd $(tools)/PaRMAT/Release && make RMAT_Generator

ANALYSIS:
	cd $(tools)/analysis && make run

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