FILE_NAME       = rmat
BATCH_SIZE      = 100000
BATCH_TIME      = 1
SNAP_VERTEX_NUM = 32768
SNAP_EDGE_NUM   = 524288

OUTPUT_STD = ~/tmp/output_std/pr_output
OUTPUT     = ~/tmp/output1/pr_output
DIFF			 = ~/tmp/diff/pr_output
CORE_NUM   = 52

tools = ./tools
inputs = ./inputs
apps = ./apps

.PHONY: Snap2Adj Generator PageRank PRCompare 

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF CORE_NUM

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