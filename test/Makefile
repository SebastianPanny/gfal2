#
#  Copyright (C) 2004 by CERN
#

LFC_DIR=$(prefix)/opt/lcg
GFAL_DIR=..
#CHECK_DIR=/usr

CHECK_DIR=/afs/cern.ch/user/j/jamesc/tmp1/lcg-deps/check
#
#

CFLAGS += -g -fPIC -pthread -D_REENTRANT -Wall

SHLIBLDFLAGS = -shared

INCLUDES= -I$(LFC_DIR)/include/lfc -I$(GFAL_DIR) -I$(CHECK_DIR)/include

LIBS = -L$(CHECK_DIR)/lib -lcheck -L$(GFAL_DIR) -lgfal


all: gfal-test

catalog_tests.o: catalog_tests.c
	$(CC) $(CFLAGS) -c $(INCLUDES) catalog_tests.c

getbestfile_tests.o: getbestfile_tests.c
	$(CC) $(CFLAGS) -c $(INCLUDES) getbestfile_tests.c

test_helpers.o: test_helpers.c
	$(CC) $(CFLAGS) -c $(INCLUDES) test_helpers.c

test_harness.o: test_harness.c
	$(CC) $(CFLAGS) -c $(INCLUDES) test_harness.c

gfal-test : test_harness.o  catalog_tests.o getbestfile_tests.o test_helpers.o $(GFAL_DIR)/libgfal.so
	$(CC) -o gfal-test test_harness.o catalog_tests.o getbestfile_tests.o test_helpers.o $(LIBS) 

test: gfal-test
	gfal-test

clean:
	rm -f *.o

clobber: clean
	rm -f gfal-test
