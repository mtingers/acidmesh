
CFLAGS=-Wall -O2
CC=cc

default: tests

tests: wordbank.o util.o forest.o
	$(CC) $(CFLAGS) util.o wordbank.o forest.c -DTEST_FOREST -o ../test_forest
	$(CC) $(CFLAGS) util.o forest.o wordbank.c -DTEST_WORDBANK -o ../test_wordbank
	cp -v ../test_forest ../bin/
	cp -v ../test_wordbank ../bin/

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

forest.o:  forest.c forest.h
	$(CC) $(CFLAGS) -c forest.c

wordbank.o:  wordbank.c wordbank.h
	$(CC) $(CFLAGS) -c wordbank.c

clean:
	rm -v *.o ../test_forest ../test_wordbank
