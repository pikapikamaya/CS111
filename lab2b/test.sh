#!/bin/bash

# NAME: Yanyin Liu

./lab2_list --threads=1 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=1 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=2 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=24 --iterations=1000 --sync=s >> lab2b_list.csv


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



