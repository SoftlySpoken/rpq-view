This is the code repository accompanying the paper titled "Materialized View Selection & View-Based Query Planning for Regular Path Queries."

## Compiling the code

```bash
$ cmake -S . -B build
$ cmake --build build -j
```



## Obtaining the data

The dataset we used is too large to upload to GitHub. It can be downloaded [here](https://www.jianguoyun.com/p/DRVlc_MQ0J6KCxjF9qYFIAA). Please decompress it and copy the contents to a newly created directory `real_data/` under the cwd `rpq-view/`:

```
$ tar -xzvf wikidata.tar.gz
$ mkdir real_data
$ mv wikidata/ real_data/
```


## Running the code

```bash
$ cd build
$ ./chooseMatViewsTheoCompare
```

Interpreting the output to `stdout`:

- "Plan time": the construction time of the AODC.
- "Naive execution time": the execution time using minimal DFA plans.
- "Choose materialized views time", "Materialize iews time": the time spent choosing and materializing views, the sum of which is the "materialize time" in Fig. 8.
- "Mode 5 execution time": the execution time using our method.
