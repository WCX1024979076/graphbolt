FILE_NAME = email-Eu-core
BATCH_SIZE = 1000
BATCH_TIME = 10

tools = ./tools
inputs = ./inputs
apps = ./apps

.PHONY: Snap2Adj Generator

export FILE_NAME BATCH_SIZE BATCH_TIME

Generator :
	cd $(tools)/updateGenerator && make run

Snap2Adj :
	cd $(tools)/converters && make run

PageRank :
	cd $(apps) && make PageRankRun

PRCompare :
	cd $(tools)/output_comparators && make PRCompare