#!/bin/bash

# NAME: Yanyin Liu
# EMAIL: yanyinliu8@gmail.com
# ID: 604952257

# lab2b_1.png.
# Mutex synchronized list operations, 1,000 iterations, 1,2,4,8,12,16,24 threads
./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv

# Spin-lock synchronized list operations, 1,000 iterations, 1,2,4,8,12,16,24 threads
./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv


# lab2b_3.png.
# Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 1, 2, 4, 8, 16 iterations 
for t in 1 4 8 12 16
do
	for i in 1 2 4 8 16
	do
		./lab2_list --threads=$t --iterations=$i --yield=id --lists=4 >> lab2b_list.csv
	done
done


#Run your program with --yield=id, 4 lists, 1,4,8,12,16 threads, and 10, 20, 40, 80 iterations, --sync=s and --sync=m
for t in 1 4 8 12 16
do
	for i in 10 20 40 80
	do
		./lab2_list --threads=$t --iterations=$i --sync=s --yield=id --lists=4 >> lab2b_list.csv
		./lab2_list --threads=$t --iterations=$i --sync=m --yield=id --lists=4 >> lab2b_list.csv
	done
done


# lab2b_4.png(symc=m) and lab2b_5.png(sync=s)
# Rerun both synchronized versions, without yields, for 1000 iterations, 1,2,4,8,12 threads, and 1,4,8,16 lists.
for t in 1 2 4 8 12
do
	for l in 4 8 16
	do
		for s in 'm' 's'
		do
			./lab2_list --threads=$t --iterations=1000 --lists=$l --sync=$s >> lab2b_list.csv
		done
	done
done



