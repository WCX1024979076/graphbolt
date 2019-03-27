# GraphBolt: Dependency-Driven Synchronous Processing of Streaming Graphs

## Requirements
- The initial input graph should be in the adjaceny graph format (http://www.cs.cmu.edu/~pbbs/benchmarks/graphIO.html).
- Edge additions and deletions file should be in SNAP format. i.e one edge in each line. The source and destination of the edge are separated by a space.
Example edge additions file:
```
1 2
2 3
...
```
- gcc version used : gcc 5.4.0.


## Running PageRank

`./PageRank_GraphBolt -numberOfUpdateBatches 2 -maxiters 10 -nAdditions 1000 -additionsFile "additionsFile.snap -nDeletions 1000 -deletionsFile "deletionsFile.snap" "inputGraph.adj"` 

### Parameters
 - nAdditions : Number of edge additions
 - nDeletions : Number of edge deletions
 - additionsFile : Input file which contains the stream of edge to be added
 - deletionsFile : Input file which contains the stream of edges to be deleted
 - numberOfUpdateBatches : Optional parameter to specify the number of edge updates to be made. Default is 1
 - maxIters : Optional parameter to specify the number of iterations that the algorithm should be run. Default is 100
 - nWorkers : Optional parameter to specify the number of worker threads. Default is specified 

## Running Label Propagation

`./LabelPropagation_GraphBolt -features 2  -seedsFile "seedsFile.txt" -numberOfUpdateBatches 2 -maxiters 10 -nAdditions 1000 -additionsFile "additionsFile.snap -nDeletions 1000 -deletionsFile "deletionsFile.snap" "inputGraph.adj"` 
### Parameters
In addition to the parameters specified in PageRank, the following additional parameters are required for running Label Propagation.
 - seedsFile : File which specify the list of seed vertices. Each seed vertex should be specified in a separate line
 - features : The number of features for each vertex


## Resources
Mugilan Mariappan and Keval Vora. [GraphBolt: Dependency-Driven Synchronous Processing of Streaming Graphs](https://dl.acm.org/citation.cfm?id=3303974). European Conference on Computer Systems (**EuroSys'19**). Dresden, Germany, March 2019.

To cite GraphBolt, you can use the following BibTeX entry:

```
@inproceedings{Mariappan:2019:GDS:3302424.3303974,
 author = {Mariappan, Mugilan and Vora, Keval},
 title = {GraphBolt: Dependency-Driven Synchronous Processing of Streaming Graphs},
 booktitle = {Proceedings of the Fourteenth EuroSys Conference 2019},
 series = {EuroSys '19},
 year = {2019},
 isbn = {978-1-4503-6281-8},
 location = {Dresden, Germany},
 pages = {25:1--25:16},
 articleno = {25},
 numpages = {16},
 url = {http://doi.acm.org/10.1145/3302424.3303974},
 doi = {10.1145/3302424.3303974},
 acmid = {3303974},
 publisher = {ACM},
 address = {New York, NY, USA},
 keywords = {Incremental Processing, Streaming Graphs},
} 
```

