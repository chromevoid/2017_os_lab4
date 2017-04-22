# 2017_os_lab4

---------------
Compile
---------------
Make sure that main.cpp and paging.h are in the same directory. Then, compile the file:
	g++ -std=c++0x main.cpp -o Paging

---------------
Run
---------------
Make sure that paging and random-numbers.txt are in the same directory. Then, run:

	./Paging M P S J N R debugging

  The meaning of each argument:
    -- M, the machine size in words.    -- P, the page size in words.    -- S, the size of a process, i.e., the references are to virtual addresses 0..S-1. 
    -- J, the job mix, which determines A, B, and C.    -- N, the number of references for each process.    -- R, the replacement algorithm, LIFO (NO T FIFO), RANDOM, or LRU.
    -- debugging, 0 for normal output, and 1 for detailed output

---------------
Sample input
---------------
./Paging 10 10 20 1 10 lru 0

---------------
Sample output
---------------
The machine size is 10.
The page size is 10.
The process size is 20.
The job mix number is 1.
The number of references per process is 10.
The replacement algorithm is lru.
The level of debugging output is 0.

Process 1 had 2 faults and 9 average residency

The total number of faults is 2 and the overall average residency is 9

