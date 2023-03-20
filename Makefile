FILE_NAME = email-Eu-core
BATCH_SIZE = 20
BATCH_TIME = 20

tools = ./tools
inputs = ./inputs
apps = ./apps

.PHONY: Snap2Adj Generator

export FILE_NAME BATCH_SIZE BATCH_TIME

Generator:
	cd $(tools)/updateGenerator && make run

Snap2Adj:
	cd $(tools)/converters && make run

PageRank:
	cd $(apps) && make PageRank
	./apps/PageRank -numberOfUpdateBatches $(BATCH_TIME) -nEdges $(BATCH_SIZE) -streamPath ./inputs/$(FILE_NAME)_operations.txt -outputFile /tmp/output/pr_output ./inputs/$(FILE_NAME)_init.adj