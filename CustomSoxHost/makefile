###############################################################################
#
# GENERAL
#

INCLUDES	=	
LIBRARIES	=	/datas/cnt/workspace/sox/sox-install/lib64/libsox.so
#-Werror
CFLAGS		=	-Wall -O3 -fPIC -std=c99
CXXFLAGS	=	$(CFLAGS)	
BIN		=	CustomSoxHost
CC		=	cc
CPP		=	c++


all:	main.o
	$(CC) $(LIBRARIES) main.o -o $(BIN)


###############################################################################
#
# PROGRAMS
#

main.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c main.c -o main.o

clean:
	rm *.o
	rm testVFX.wav
	rm CustomSoxHost

