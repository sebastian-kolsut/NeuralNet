CC       = cc
CFLAGS   = -Wall -Wextra -g
ASAN     = -fsanitize=address,undefined
# matrices.c has its own main(); rename it so the test runner's main is used
SRC_DEFS = -Dmain=matrices_c_main

SRCS = test_matrices.c ../src/matrices.c ../headers/matrices.h

all: run_tests

run_tests: $(SRCS)
	$(CC) $(CFLAGS) -c ../src/matrices.c $(SRC_DEFS) -o matrices.o
	$(CC) $(CFLAGS) test_matrices.c matrices.o -lm -o $@
	rm -f matrices.o

run_tests_asan: $(SRCS)
	$(CC) $(CFLAGS) $(ASAN) -c ../src/matrices.c $(SRC_DEFS) -o matrices_asan.o
	$(CC) $(CFLAGS) $(ASAN) test_matrices.c matrices_asan.o -lm -o $@
	rm -f matrices_asan.o

# run all tests, report pass/fail for each
test: run_tests
	./run_tests

# same tests under AddressSanitizer + UBSan (stops at first memory error)
asan: run_tests_asan
	./run_tests_asan

clean:
	rm -f run_tests run_tests_asan *.o

.PHONY: all test asan clean
