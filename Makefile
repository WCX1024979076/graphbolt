FILE_NAME = soc-sign
BATCH_SIZE = 10000
BATCH_TIME = 10
OUTPUT_STD = /tmp/output_std/pr_output
OUTPUT     = /tmp/output/pr_output
DIFF			 = /tmp/diff/pr_output

tools = ./tools
inputs = ./inputs
apps = ./apps

.PHONY: Snap2Adj Generator PageRank PRCompare 

export FILE_NAME BATCH_SIZE BATCH_TIME OUTPUT_STD OUTPUT DIFF

Generator :
	cd $(tools)/updateGenerator && make run

Snap2Adj :
	cd $(tools)/converters && make run

PageRank :
	cd $(apps) && make PageRankRun

PRCompare :
	cd $(tools)/output_comparators && make PRCompare