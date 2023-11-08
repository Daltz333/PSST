# Define compiler and flags
CC = gcc
CFLAGS = -Wall

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

MAILBOX_DIR = $(SRC_DIR)/mailbox
AUTH_DIR = $(SRC_DIR)/auth
PSST_DIR = $(SRC_DIR)/psst
SHARED_DIR = $(SRC_DIR)/shared

SHARED_SRC := $(shell find $(SHARED_DIR) -name '*.c' -o -name '*.h')

MAILBOX_SRC := $(shell find $(MAILBOX_DIR) -name '*.c' -o -name '*.h') $(SHARED_SRC)
AUTH_SRC := $(shell find $(AUTH_DIR) -name '*.c' -o -name '*.h') $(SHARED_SRC)
PSST_SRC := $(shell find $(PSST_DIR) -name '*.c' -o -name '*.h') $(SHARED_SRC)

# Object files for each target
MAILBOX_OBJ = $(patsubst $(MAILBOX_DIR)/%.c, $(OBJ_DIR)/%.o, $(MAILBOX_SRC))
AUTH_OBJ = $(patsubst $(AUTH_DIR)/%.c, $(OBJ_DIR)/%.o, $(AUTH_SRC)) 
PSST_OBJ = $(patsubst $(PSST_DIR)/%.c, $(OBJ_DIR)/%.o, $(PSST_SRC)) 

# Targets
all: mailbox psst auth

mailbox: $(MAILBOX_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

psst: $(PSST_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

auth: $(AUTH_OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $(BIN_DIR)/$@ $^

$(OBJ_DIR)/%.o: $(MAILBOX_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(AUTH_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(PSST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

clean:
	rm -rf $(BIN_DIR) $(OBJ_DIR)

.PHONY: all auth mailbox psst clean
