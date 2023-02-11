# Executables.
CC?=gcc
COMPILEDB?=compiledb

# Flags.
CFLAGS+=-Wall -O0 -g

# Define directories.
PWD:=$(shell pwd)
GEN_DIR:=$(PWD)/gen
ROFL_DIR:=$(PWD)/rofl
TEST_DIR:=$(PWD)/test
INC_DIRS:=$(ROFL_DIR)

# Define libraries.
SHARED_LIBS:=

# Define tool files.
TEST_HEADERS:=$(wildcard $(TEST_DIR)/*.h)
TEST_SRCS:=$(wildcard $(TEST_DIR)/*.c)
TEST_OBJS:=$(TEST_SRCS:$(TEST_DIR)/%.c=$(GEN_DIR)/_TEST_%.o)
TEST_OUT:=$(GEN_DIR)/rofl-test

# Define game files.
ROFL_HEADERS:=$(wildcard $(ROFL_DIR)/*.h)
ROFL_SRCS:=$(wildcard $(ROFL_DIR)/*.c)
ROFL_OBJS:=$(ROFL_SRCS:$(ROFL_DIR)/%.c=$(GEN_DIR)/_ROFL_%.o)


# Phony targets.
default: compiledb

.PHONY: \
	test \
    clean \
	compiledb \
    default

test: $(TEST_OUT)
	$(TEST_OUT)

compiledb:
	$(COMPILEDB) -n make

clean:
	rm -f $(TEST_OUT) $(ROFL_OBJS) $(TEST_OBJS)

# Targets.
# Executables
$(TEST_OUT): $(TEST_OBJS) $(ROFL_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(INC_DIRS:%=-I%) $(SHARED_LIBS:%=-l%)

# Object files.
$(GEN_DIR)/_TEST_%.o: $(TEST_DIR)/%.c $(TEST_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_DIRS:%=-I%)

$(GEN_DIR)/_ROFL_%.o: $(ROFL_DIR)/%.c $(ROFL_HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@ $(INC_DIRS:%=-I%)
