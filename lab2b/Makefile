# NAME: Yanyin Liu
FILES = lab2_list.c Makefile README SortedList.h SortedList.c lab2b_list.csv *.png profile.out test.sh *.gp
FILES2 = lab2_list *.tar.gz
#FILES2 = lab2_list *.tar.gz *.png lab2b_list.csv
default:	build

build:
	@gcc -pthread -Wall -Wextra -g -lprofiler lab2_list.c -o lab2_list

tests:
	@chmod +x test.sh
	-@./test.sh

profile: 
	-rm -f ./raw.gperf
	LD_PRELOAD=/usr/lib64/libprofiler.so.0 CPUPROFILE=raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf>profile.out
	pprof --list=func ./lab2_list ./raw.gperf>>profile.out
	rm -f ./raw.gperf

graphs:
	@gnuplot lab2_list.gp

dist:
	@make build
	@make tests
	@make graphs
	@make profile
	@tar -czf lab2b-604952257.tar.gz $(FILES)

clean:
	@rm -f $(FILES2)
	
 
