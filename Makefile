CC = g++
CFLAGS = -Wall -Wextra -g 
DFLAGS = -DDebug -g3 -O0 -Werror -Wall
INCLUDES = 
LFLAGS =
LIBS = -lm -lpthread
SRCS = tema1.cpp mapreduce.cpp inparser.cpp
OBJS = $(SRCS:.c=.o)

MAIN = tema1

.PHONY: depend clean

build: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

build_debug:
	$(CC) $(DFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)
	$(RM) peda-*
	$(RM) valgrind-*$

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it