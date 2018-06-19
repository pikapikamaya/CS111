# NAME: Yanyin Liu
# EMAIL: yanyinliu8@gmail.com
# ID: 604952257
FILES = lab1b-client.c lab1b-server.c Makefile README

default:
	@gcc -g -lz -Wall -Wextra lab1b-client.c -o lab1b-client
	@gcc -g -lz -Wall -Wextra lab1b-server.c -o lab1b-server

# In order to le the make check included clean, I put it before the check rule
clean:
	@rm -f lab1b-client lab1b-server *.tar.gz

dist:
	@tar -czf lab1b-604952257.tar.gz $(FILES)

