FILE_NAME = soc-LiveJournal1
BATCH_SIZE = 100000
BATCH_TIME = 10
OUTPUT_STD = ~/tmp/output_std/pr_output
OUTPUT     = ~/tmp/output1/pr_output
DIFF			 = ~/tmp/diff/pr_output
CORE_NUM   = 4

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
