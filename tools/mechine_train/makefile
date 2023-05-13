.PHONY: TRAIN PREDICT EVAL

TRAIN:
	python3 train.py

PREDICT:
	$(eval GRAPHBOLT_ITER := $(shell python3 predict.py))
	export GRAPHBOLT_ITER
	echo $(GRAPHBOLT_ITER)

EVAL:
	python3 eval.py