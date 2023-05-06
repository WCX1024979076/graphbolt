PWD = $(shell pwd)
FILE_NAME = rmat
BATCH_SIZE = 100000
BATCH_TIME = 10
SNAP_VERTEX_NUM = 100000
SNAP_EDGE_NUM = 1000000
SNAP_FILE = $(PWD)/inputs/$(FILE_NAME).snap
OUTPUT_STD = ~/tmp/output_std/pr_output
OUTPUT     = ~/tmp/output1/pr_output
DIFF			 = ~/tmp/diff/pr_output
CORE_NUM   = 4

tools = ./tools
inputs = ./inputs
apps = ./apps

.PHONY: Snap2Adj Generator PageRank PRCompare 

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF CORE_NUM SNAP_FILE SNAP_VERTEX_NUM SNAP_EDGE_NUM

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

