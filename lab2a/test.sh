#!/bin/bash

# NAME: Yanyin Liu


# Run your program for ranges of iterations (100, 1000, 10000, 100000) values (with no thread and no yield)
./lab2_add --iterations=100 >> lab2_add.csv
./lab2_add --iterations=1000 >> lab2_add.csv
./lab2_add --iterations=10000 >> lab2_add.csv
./lab2_add --iterations=100000 >> lab2_add.csv

# Re-run your tests, with yields, for ranges of threads (2,4,8,12) and iterations (10, 20, 40, 80, 100, 1000, 10000, 100000)
# lab2_add-1.png
./lab2_add --threads=2 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv

./lab2_add --threads=4 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=4 --iterations=100000 --yield >> lab2_add.csv

./lab2_add --threads=8 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 --yield >> lab2_add.csv

./lab2_add --threads=12 --iterations=10 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=20 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=40 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=80 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=12 --iterations=100000 --yield >> lab2_add.csv

# threads (2, 8) and of iterations (100, 1000, 10000, 100000), non yield vs with yield
# lab2_add-2.png
./lab2_add --threads=2 --iterations=100 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=2 --iterations=100000 --yield >> lab2_add.csv

./lab2_add --threads=8 --iterations=100 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=1000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=10000 --yield >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 >> lab2_add.csv
./lab2_add --threads=8 --iterations=100000 --yield >> lab2_add.csv

#synchronization option (none, mutex, spin, compare-and-swap), with yield
#iterations (10,000 for mutexes and CAS, only 1,000 for spin locks)
#lab2_add-4.png
for t in 2 4 8 12
do
	./lab2_add --threads=$t --iterations=10000 --yield --sync=m >> lab2_add.csv
	./lab2_add --threads=$t --iterations=10000 --yield --sync=c >> lab2_add.csv
	./lab2_add --threads=$t --iterations=1000 --yield --sync=s >> lab2_add.csv
done

#for a range of number of threads (1,2,4,8,12), no yield, large enough number of iterations (e.g. 10,000) t
#lab2_add-5.png
for t in 1 2 4 8 12
do
	for s in 'm' 's' 'c'
	do
		./lab2_add --threads=$t --iterations=10000 --sync=$s >> lab2_add.csv
	done
	
done




# with a single thread, and increasing numbers of iterations (10, 100, 1000, 10000, 20000)
# lab2_list-1.png
./lab2_list --threads=1  --iterations=10 >> lab2_list.csv
./lab2_list --threads=1  --iterations=100 >> lab2_list.csv
./lab2_list --threads=1  --iterations=1000 >> lab2_list.csv
./lab2_list --threads=1  --iterations=10000	>> lab2_list.csv
./lab2_list --threads=1  --iterations=20000	>> lab2_list.csv

 #using various combinations of yield options and see how many threads (2,4,8,12) and iterations (1, 2,4,8,16,32)
for i in 1 2 4 8 16 32
do
	for t in 2 4 8 12
	do
		for y in 'i' 'id' 'idl' 'il' 'dl' 'd' 'l'
		do
			./lab2_list --threads=$t --iterations=$i --yield=$y >> lab2_list.csv
		done
	done
done

# pthread_mutexes (--sync=m), and another protected by test-and-set spin locks (--sync=s).
# numbers of threads (2) and iterations (16)
# numbers of threads (12) and iterations (32)
# lab2_list-3.png.

./lab2_list --threads=2 --iterations=16 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=il --sync=m >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=dl --sync=m >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=il --sync=s >> lab2_list.csv
./lab2_list --threads=2 --iterations=16 --yield=dl --sync=s >> lab2_list.csv

./lab2_list --threads=12 --iterations=32 --yield=i  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=i  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=d  --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=il --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=32 --yield=dl --sync=s >> lab2_list.csv

# iterations (e.g. 1000), threads (1, 2, 4, 8, 12, 16, 24), synchronization options: mutex, spin
# lab2_list-4.png
./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2_list.csv

./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2_list.csv



