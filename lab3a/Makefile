#NAME: Yanyin Liu,Shawye Ho


#Makefile

CC=gcc
#header file named
EXT=ext2_fs.h

#flags
CFLAGS= -g -Wall -Wextra

#executable named lab3a
TARGET=lab3a

#tarball named lab3a.tar.gz
TAR=lab3a-704839317.tar.gz

all: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

dist:
	tar -zvcf $(TAR) $(TARGET).c Makefile README $(EXT)

clean:
	$(RM) $(TARGET) $(TAR)
