This is the code repository accompanying the paper titled "Materialized View Selection & View-Based Query Planning for Regular Path Queries", accepted by SIGMOD 2024. Authors: Yue Pang (PKU), Lei Zou (PKU), Jeffrey Xu Yu (CUHK), Linglin Yang (PKU).

## Compiling the code

```bash
$ cmake -S . -B build
$ cmake --build build -j
```

## Obtaining the data

The dataset we used is too large to upload to GitHub. It can be downloaded [here](https://www.jianguoyun.com/p/DRVlc_MQ0J6KCxjF9qYFIAA). Please decompress it and copy the contents to a newly created directory `real_data/` under the cwd `rpq-view/`:

```bash
$ tar -xzvf wikidata.tar.gz
$ mkdir real_data
$ mv wikidata/ real_data/
```

## Running the code

All the following commands are run under the `build` directory unless otherwise specified.

### Effectiveness of Query Planning with AODC (Section 8.2)

```bash
$ ./CompareAndOrDagDfa
```

Interpreting the output to `stdout`:

- "AND-OR DAG execution used": the query execution time using our proposed AODC query plans.
- "DFA execution used": the query execution time using the minimal DFA plans.

### Proof of Concept: MVS for RPQ (Section 8.3)

```bash
$ ./matMostFrequent
```

Interpreting the output to `stdout`:

- "Total time": the query execution time using the materialized views selected by the naive MVS algorithm. (In the query, this time is compared with that of 【】)

### Impact of the Memory Budget (Section 8.4)

```bash
$ ./varyMemBudget
```

Interpreting the output to `stdout`:

- "Budget x execution time": the query execution time using the materialized views selected with budget = x. In our experiments, $x\in\{10^6, 10^8, 10^9\}$ are tested.

### Impact of the Workload Size (Section 8.5)

```bash
$ ./varyWorkloadSz
```

Interpreting the output to `stdout`:

- "queryFilePath": the currently running sampled workload.
- "Naive execution time": the current workload's query execution time without materialized views.
- "Execution time": the current workload's query execution time with the materialized views selected by our proposed method.

### Comparison with State-of-the-Art (Section 8.6)

#### Our method

```bash
$ ./ours 5
```

5 is the execution mode corresponding to our method.

Interpreting the output to `stdout`:

- "Execution time": the query execution time.

#### Competitor: RTC

```bash
$ ./ours 4
```

5 is the execution mode corresponding to RTC.

#### Competitor: Swarmguide

Swarmguide's method consists of the following stages:

1. Affinity propagation by edge label to cluster the queries:

    ```bash
    $ ./ap <num_iter> <damping_factor>
    ```

    In our paper's experiments, `num_iter` is fixed as 100, and `damping_factor` is varied across 0.2, 0.4, 0.6, and 0.8. The outputs are preserved as files named `ap_<num_iter>_<damping_factor>` under the `ap_output` directory.

    The source code is modified from [this repository](https://github.com/jincheng9/AffinityPropagation).

2. Select the maximum common sub-automaton of each cluster as the materialized views. We first use the following program to determine whether there exists a common sub-automaton for each cluster:

    ```bash
    $ ./productGraphQueryBatch <ap_output_file>
    ```

    The outputs are preserved as files named `ap_<num_iter>_<damping_factor>-view` under the `ap_output` directory.

    Since most of the clusters do not have common sub-automata, we manually extract the maximum common sub-automaton of each remaining cluster. (An external solver can also be used for this; just follow the output file format.) The outputs are preserved as files named `ap_<num_iter>_<damping_factor>-view-manual` under the `ap_output` directory.

3. Materialize the selected views and run the workload queries with them:

    ```bash
    $ ./swarmGuide <num_iter> <damping_factor>
    ```

    Interpreting the output to `stdout`:

    - "Total time": the query execution time.

#### Competitor: Ring

In the paper's experiments, we used the code repository published by Ring's authors: 

Note that we use the problem defintion where neither of the endpoints of the RPQ is bounded to a constant, so our workload is different from that in Ring's paper [1].

[1] D. Arroyuelo, A. Hogan, G. Navarro, and J. Rojas-Ledesma, “Time- and Space-Efficient Regular Path Queries,” in *2022 IEEE 38th International Conference on Data Engineering (ICDE)*, Kuala Lumpur, Malaysia: IEEE, May 2022, pp. 3091–3105. doi: [10.1109/ICDE53745.2022.00277](https://doi.org/10.1109/ICDE53745.2022.00277).

#### Competitor: Unit

```bash
$ cd build
$ ./unitCost
```

Interpreting the output to `stdout`:

The last line: exeTime planExeTime, where

- exeTime is the query execution time, and
- planExeTime is the planning + query execution time.

In the paper, only the query execution time is accounted for.



```bash
$ cd build
$ ./chooseMatViewsTheoCompare --execute
```

Interpreting the output to `stdout`:

- "Plan time": the construction time of the AODC.
- "Naive execution time": the execution time using minimal DFA plans.
- "Choose materialized views time", "Materialize iews time": the time spent choosing and materializing views, the sum of which is the "materialize time" in Fig. 8.
- "Mode 5 execution time": the execution time using our method.

## Citing the paper

ACM Reference Format:

Yue Pang, Lei Zou, Jeffrey Xu Yu, and Linglin Yang. 2024. Materialized View Selection & View-Based Query Planning for Regular Path Queries. Proc. ACM Manag. Data 2, 3 (SIGMOD), Article 152 (June 2024), 26 pages. https://doi.org/10.1145/3654955
