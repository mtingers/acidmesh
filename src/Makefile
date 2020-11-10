
CFLAGS=-Wall -O3
CC=cc

default: tests

debug: datatree.o util.o mesh.o container.o
	$(CC) $(CFLAGS) util.o context.o datatree.o container.o mesh.c -DTEST_FOREST -D DEBUG -o ../bin/test_mesh

tests: datatree.o util.o context.o mesh.o container.o
	$(CC) $(CFLAGS) util.o context.o datatree.o container.o mesh.c -DTEST_FOREST  -o ../bin/test_mesh

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

context.o: context.c container.h context.h util.h mesh.h datatree.h
	$(CC) $(CFLAGS) -c context.c

container.o: container.c container.h context.h util.h mesh.h datatree.h
	$(CC) $(CFLAGS) -c container.c

mesh.o:  mesh.c container.h context.h util.h mesh.h datatree.h
	$(CC) $(CFLAGS) -c mesh.c

datatree.o:  datatree.c container.h context.h util.h mesh.h datatree.h
	$(CC) $(CFLAGS) -c datatree.c

py:
	rm -r build || echo
	python setup.py build
	python setup.py install

clean:
	rm -r build || echo
	rm -v *.o 
