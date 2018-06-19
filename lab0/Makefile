# NAME: Yanyin Liu
# EMAIL: yanyinliu8@gmail.com
# ID: 604952257

FILES = lab0.c Makefile README *.png

default:
	@gcc -g -Wall -Wextra lab0.c -o lab0

# In order to le the make check included clean, I put it before the check rule
clean:
	@rm -f lab0 *.o *.txt *.tar.gz

dist:
	@tar -czf lab0-604952257.tar.gz $(FILES)

check:
	@make clean
	@make default
	# When a line starts with ‘@’, the echoing of that line is suppressed.
	# $$? tells make that $? is a variable in the shell 
	# check if copying files with correct exit code
	@echo "Hello! We will use this to be the input file to check!" > input1.txt;
	@./lab0 --input=input1.txt --output=output1.txt; \
	if [ $$? -eq 0 ]; \
	then \
		echo "Copy the input1.txt to output1.txt with correct exit code."; \
	else \
		echo "Ooh... Copy the input1.txt to output1.txt with correct exit code."; \
	fi

	# check if copying files correctly by checking if two files are the same
	# check the files byte by byte
	@cmp input1.txt output1.txt; \
	if [ $$? -ne 0 ]; \
	then \
		echo "Ooh...Did not copy the files correctly. input1.txt and output1.txt are different!"; \
	else \
		echo "Copy the files correctly! input1.txt and output1.txt are the same!"; \
	fi

	# Check if handle the unrecognized argument correctly
	@./lab0 --testit=input1.txt --tryit=output1.txt; \
	if [ $$? -ne 1 ]; \
	then \
		echo "Ooh...Unrecognized aargument is found but with wrong exit code."; \
	else \
		echo "Handle the unrecognized argument correctly!"; \
	fi

	# Check if handle the case that unable to open input file correctly
	@echo "Hello! We will use this to be the unopenable input file!" > input2.txt; 
	@ chmod -r input2.txt;
	@./lab0 --input=input2.txt > outputUnopenable.txt; \
	if [ $$? -ne 2 ]; \
	then \
		echo "Ooh...Handle the unopenable input file with wrong exit code."; \
	else \
		echo "Handle the unopenable input file correctly."; \
	fi

	# Check if handle the case that unable to open/create output file correctly
	@echo "Hello! We will use this to be the unopenable/ uncreatable output file!" > output2.txt; 
	@ chmod -w output2.txt;
	@./lab0 --output=output2.txt < input1.txt; \
	if [ $$? -ne 3 ]; \
	then \
		echo "Ooh...Handle the unopenable/ uncreatable output file with wrong exit code."; \
	else \
		echo "Handle the unopenable/ uncreatable file correctly."; \
	fi

	# Check if caught and received SIGSEGV correctly
	@./lab0 --segfault --catch; \
	if [ $$? -eq 4 ]; \
	then \
		echo "Caught and received SIGSEGV correctly."; \
	else \
		echo "Ooh... Fail to handle catch and receive SIGSEGV."; \
	fi


