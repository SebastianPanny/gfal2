#
#  Copyright (C) 2004 by CERN
#

#LFC_DIR=/home/jamesc/work/LFC
LFC_DIR=$(prefix)/opt/lcg
GFAL_DIR=..

#
#

CC += -g -fPIC
SHLIBLDFLAGS = -shared

INCLUDES= -I$(LFC_DIR)/include -I$(GFAL_DIR)

LIBS = -lcheck -L$(GFAL_DIR) -lgfal


all: gfal-test

catalog_tests.o: catalog_tests.c
	$(CC) -c $(INCLUDES) catalog_tests.c

test_helpers.o: test_helpers.c
	$(CC) -c $(INCLUDES) test_helpers.c

test_harness.o: test_harness.c
	$(CC) -c $(INCLUDES) test_harness.c

gfal-test : test_harness.o  catalog_tests.o test_helpers.o $(GFAL_DIR)/libgfal.so
	$(CC) -o gfal-test test_harness.o catalog_tests.o test_helpers.o $(LIBS) 

test: gfal-test
	gfal-test

clean:
	rm -f *.o

clobber: clean
	rm -f gfal-test
