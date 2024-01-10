# OS Kernel Scheduler Simulator

The base of the main.c script was provided by Prof. Wainer. Claire Villanueva and Divya Vithiyatharan expanded on the
code to implement different scheduler types and memory management.


## Setup
Required environment: Linux

To compile the program, open the Linux terminal inside the project folder and use the following command:
```bash
gcc -o assignment2.o main.c
```

## Simulating the Schedulers
This program can simulate three schedulers, First Come First Serve, External Priorities, and Round Robin.
To run these schedulers, there are different configurations for the desired input/output.

For the computation analysis tests, open the terminal inside the metrics_testcases folder and perform a similar procedure as the examples below, but simply with "test1.sh" i.e.

### Below are some examples of how to output to the output.csv:

#### FCFS without I/O
```bash
bash fcfs_non-verbose.sh
```
#### External Priorities with I/O
```bash
bash priority_io_non-verbose.sh
```
#### Round Robin without I/O
```bash
bash rr_non-verbose.sh
```

### Below are some examples of how to output to the terminal:
#### FCFS with I/O
```bash
bash fcfs_io_verbose.sh
```
#### External Priorities without I/O
```bash
bash priority_verbose.sh
```
#### Round Robin with I/O
```bash
bash rr_io_verbose.sh
```

### Simulating Memory Management
#### Managing Memory (500 Mb, 250 Mb, 150 Mb, 100 Mb); Output should have this format: mm_test*_output.csv
```bash
bash memory_management_s3_test1.sh
```
```bash
bash memory_management_s3_test2.sh
```
```bash
bash memory_management_s3_test3.sh
```
```bash
bash memory_management_s3_test4.sh
```
```bash
bash memory_management_s3_test5.sh
```
#### Managing Memory (300 Mb, 300 Mb, 350 Mb, 50 Mb); Output should have this format: mm_test*_output.csv
```bash
bash memory_management_s4_test6.sh
```
```bash
bash memory_management_s4_test7.sh
```
```bash
bash memory_management_s4_test8.sh
```
```bash
bash memory_management_s4_test9.sh
```
```bash
bash memory_management_s4_test10.sh
```
