# NAME: Yanyin Liu


default:
	gcc -Wall -Wextra -lmraa -lm -g -o lab4c_tcp lab4c_tcp.c
	gcc -Wall -Wextra -lmraa -lm -lssl -lcrypto -g -o lab4c_tls lab4c_tls.c

clean:
	rm -f lab4c_tcp lab4c_tls *.txt *.tar.gz

dist:
	tar -czf lab4c-604952257.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
	
