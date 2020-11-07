
CFLAGS=-Wall -O3
CC=cc

default: tests

debug: wordbank.o util.o forest.o container.o
	$(CC) $(CFLAGS) util.o wordbank.o container.o forest.c -DTEST_FOREST -D DEBUG -o ../bin/test_forest

tests: wordbank.o util.o forest.o container.o
	$(CC) $(CFLAGS) util.o wordbank.o container.o forest.c -DTEST_FOREST  -o ../bin/test_forest

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

container.o: container.c container.h
	$(CC) $(CFLAGS) -c container.c

forest.o:  forest.c forest.h
	$(CC) $(CFLAGS) -c forest.c

wordbank.o:  wordbank.c wordbank.h
	$(CC) $(CFLAGS) -c wordbank.c

clean:
	rm -v *.o 
