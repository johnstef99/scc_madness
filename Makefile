#
# Created by:   github.com/johnstef99
# Last updated: 2022-11-22
#

SRC_DIR  = ./src
OBJ_DIR  = ./obj
LIB_DIR  = ./libs

CC       = xcrun /opt/opencilk/bin/clang
CFILES   = $(wildcard $(SRC_DIR)/*.c)
LIBFILES   = $(wildcard $(LIB_DIR)/*.c)
OBJFILES = $(CFILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o) $(LIBFILES:$(LIB_DIR)/%.c=$(OBJ_DIR)/%.o)
OUT      = ./bin/scc

CFLAGS = -fopencilk -O3

all: $(OUT)

debug: CFLAGS = -fopencilk -fsanitize=cilk -Og -g -D_FORTIFY_SOURCE=0 
debug: $(OUT)

$(OUT): 
	$(CC) $(CFLAGS) -o $@ $(LIBFILES) $(CFILES)

.PHONY: clean test run
run:
	./bin/scc ./assets/eu-2005.mtx

clean:
	rm -f $(OUT)
