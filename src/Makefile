
CFLAGS=-Wall -O3
CC=cc

default: tests

debug: wordbank.o util.o forest.o container.o
	$(CC) $(CFLAGS) util.o context.o wordbank.o container.o forest.c -DTEST_FOREST -D DEBUG -o ../bin/test_forest

tests: wordbank.o util.o context.o forest.o container.o
	$(CC) $(CFLAGS) util.o context.o wordbank.o container.o forest.c -DTEST_FOREST  -o ../bin/test_forest

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

context.o: context.c container.h context.h util.h forest.h wordbank.h
	$(CC) $(CFLAGS) -c context.c

container.o: container.c container.h context.h util.h forest.h wordbank.h
	$(CC) $(CFLAGS) -c container.c

forest.o:  forest.c container.h context.h util.h forest.h wordbank.h
	$(CC) $(CFLAGS) -c forest.c

wordbank.o:  wordbank.c container.h context.h util.h forest.h wordbank.h
	$(CC) $(CFLAGS) -c wordbank.c

py:
	rm -r build || echo
	python setup.py build
	python setup.py install

clean:
	rm -r build || echo
	rm -v *.o 
