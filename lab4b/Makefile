# NAME: Yanyin Liu
# EMAIL: yanyinliu8@gmail.com
# ID: 604952257

.SILENT:

default:
	gcc -Wall -Wextra -g -lmraa -lm -o lab4b lab4b.c
	
check: default
	{ echo "PERIOD=5"; sleep 5; echo "SCALE=C"; sleep 5; echo "SCALE=F"; sleep 2; echo "OFF"; } | ./lab4b --period=2 --scale=C --log="smoke_log.txt"
	echo "Test passed: check on PERIOD and SCALE"
	{ echo "STOP"; sleep 5; echo "START"; sleep 5; echo "OFF"; } | ./lab4b --period=2 --scale=C --log="smoke_log.txt"
	echo "Test passed: check on STOP and START"
	{ echo "STOP"; sleep 5; echo "LOG stopping"; sleep 5; echo "START"; sleep 5; echo "LOG starting"; sleep 5; echo "OFF"; } | ./lab4b --period=2 --scale=C --log="smoke_log.txt"
	echo "Test passed: check on LOG"
#	rm -f smoke_log.txt

clean:
	rm -f lab4b *.txt *.tar.gz

dist:
	tar -czf lab4b-604952257.tar.gz lab4b.c Makefile README
	
