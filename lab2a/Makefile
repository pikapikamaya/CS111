# NAME: Yanyin Liu


FILES = lab2_add.c lab2_list.c Makefile README SortedList.h SortedList.c *.csv *.png test.sh *.gp
#FILES2 = lab2_add lab2_list *.tar.gz
FILES2 = lab2_add lab2_list lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png *.tar.gz

build:
	@gcc -pthread -Wall -Wextra lab2_add.c -o lab2_add
	@gcc -pthread -Wall -Wextra lab2_list.c -o lab2_list

tests:
	@chmod +x test.sh
	@-./test.sh
graphs:
	@gnuplot lab2_add.gp
	@gnuplot lab2_list.gp

clean:
	@rm -f $(FILES2)

dist:
	@make clean
	@make build
	@make tests
	@make graphs
	@tar -czf lab2a-604952257.tar.gz $(FILES)
 
