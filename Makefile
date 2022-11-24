#
# Created by:   github.com/johnstef99
# Last updated: 2022-11-24
#

SRC_DIR  = ./src
OBJ_DIR  = ./obj
LIB_DIR  = ./libs

CC       = /opt/homebrew/opt/llvm/bin/clang
CFILES   = $(wildcard $(SRC_DIR)/*.c)
LIBFILES = $(wildcard $(LIB_DIR)/*.c)
OBJFILES = $(CFILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) $(LIBFILES:$(LIB_DIR)/%.c=$(OBJ_DIR)/%.o)
OUT      = ./bin/scc

CFLAGS = -Wall -O3 -I/opt/homebrew/opt/llvm/include -Xclang -fopenmp
LDFLAGS = -L/opt/homebrew/opt/llvm/lib -lomp

$(OUT): $(OBJFILES)
	$(CC) $(LDFLAGS) -o $@ $^ 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean test run
run:
	# OMP_NUM_THREADS=4
	./bin/scc assets/wiki-topcats.mtx

clean:
	rm -f $(OBJFILES) $(OUT)

test:
	echo $(OBJFILES)
