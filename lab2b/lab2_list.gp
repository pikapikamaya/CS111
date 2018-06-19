#! /usr/bin/gnuplot
#
# NAME: Yanyin Liu
# EMAIL: yanyinliu8@gmail.com
# ID: 604952257
#
# purpose:
#     generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#    1. test name
#    2. # threads
#    3. # iterations per thread
#    4. # lists
#    5. # operations performed (threads x iterations x (ins + lookup + delete))
#    6. run time (ns)
#    7. run time per operation (ns)
#    8. average wait-for-lock time
#
# output:
#    lab2_list-1.png ... cost per operation vs threads and iterations
#    lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#    lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#    lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#    Managing data is simplified by keeping all of the results in a single
#    file.  But this means that the individual graphing commands have to
#    grep to select only the data they want.
#
#    Early in your implementation, you will not have data for all of the
#    tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# lab2b_1.png
# To get the throughput, divide one billion (number of nanoseconds in a second) by
# the time per operation (in nanoseconds)
set title "Scalability-1: Throughput of Synchronized Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'
# grep out only single threaded, un-protected, non-yield results
plot \
"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'Mutex synchronized list operations' with linespoints lc rgb 'red', \
"< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'Spin-lock synchronized list operations' with linespoints lc rgb 'green'



# lab2b_2.png
#  plot the wait-for-lock time, and the average time per operation against the number of competing threads
set title "Scalability-2: Per-operation Times for Mutex-Protected List Operations"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "mean time/operations (ns)"
set logscale y 10
set output 'lab2b_2.png'
# note that unsuccessful runs should have produced no output
plot \
"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
title 'average completion time' with linespoints lc rgb 'red', \
"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
title 'average wait-for-lock time' with linespoints lc rgb 'green'



# lab2b_3.png
set title "Scalability-3: Correct Synchronization of Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "successful iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
"< grep list-id-none lab2b_list.csv" using ($2):($3) \
title 'unprotected' with points lc rgb 'red', \
"< grep list-id-m lab2b_list.csv" using ($2):($3) \
title 'Mutex' with points lc rgb 'green', \
"< grep list-id-s lab2b_list.csv" using ($2):($3) \
title 'Spin-Lock' with points lc rgb 'blue'



# lab2b_4.png
set title "Scalability-4: Throughput of Mutex-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_4.png'

plot \
"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=1' with linespoints lc rgb 'red', \
"< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=4' with linespoints lc rgb 'green', \
"< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=8' with linespoints lc rgb 'blue', \
"< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=16' with linespoints lc rgb 'orange'



# lab2b_5.png
set title "Scalability-5: Throughput of Spin-Lock-Synchronized Partitioned Lists"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_5.png'

plot \
"< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=1' with linespoints lc rgb 'red', \
"< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=4' with linespoints lc rgb 'green', \
"< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=8' with linespoints lc rgb 'blue', \
"< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000)/($7) \
title 'lists=16' with linespoints lc rgb 'orange'





