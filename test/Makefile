#
#  Copyright (C) 2004 by CERN
#

LFC_INC=$(prefix)/opt/lcg/include/lfc
GFAL_DIR=..
#CHECK_DIR=/usr
CHECK_DIR=/afs/cern.ch/user/j/jamesc/tmp1/lcg-deps/check
#
#

CFLAGS += -g -fPIC -pthread -D_REENTRANT -Wall

SHLIBLDFLAGS = -shared

INCLUDES= -I$(LFC_INC) -I$(GFAL_DIR) -I$(CHECK_DIR)/include

LIBS = -L$(CHECK_DIR)/lib -lcheck -L$(GFAL_DIR) -lgfal


all: gfal-unit-test-suite

catalog_tests.o: catalog_tests.c
	$(CC) $(CFLAGS) -c $(INCLUDES) catalog_tests.c

getbestfile_tests.o: getbestfile_tests.c
	$(CC) $(CFLAGS) -c $(INCLUDES) getbestfile_tests.c

test_helpers.o: test_helpers.c
	$(CC) $(CFLAGS) -c $(INCLUDES) test_helpers.c

test_harness.o: test_harness.c
	$(CC) $(CFLAGS) -c $(INCLUDES) test_harness.c

gfal-unit-test-suite : test_harness.o  catalog_tests.o test_helpers.o getbestfile_tests.o $(GFAL_DIR)/libgfal.so
	$(CC) -o gfal-unit-test-suite test_harness.o catalog_tests.o test_helpers.o getbestfile_tests.o $(LIBS) 

test: gfal-unit-test-suite
	gfal-unit-test-suite

clean:
	rm -f *.o

clobber: clean
	rm -f gfal-unit-test-suite
