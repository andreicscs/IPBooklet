# this makefile was generated using an LLM.

CC = gcc
CFLAGS = -I./include -Wall -Wextra -g -pthread
LDFLAGS = -lpthread

# Directories and Files
BIN = bin
SRC = src
APPS = apps
TESTS = tests

# Source Groups
COMMON_SRC = $(SRC)/IPBparser.c $(SRC)/IPBnetwork.c $(SRC)/IPBtypes.c
SERVER_SRC = $(APPS)/serverMain.c $(COMMON_SRC) $(SRC)/IPBserverData.c
CLIENT_SRC = $(APPS)/clientMain.c $(COMMON_SRC) $(SRC)/IPBclientAPI.c
TEST_SRC   = $(TESTS)/IPBtests.c  $(COMMON_SRC) $(SRC)/IPBclientAPI.c

.PHONY: all clean run-tests

# Default Target
all: $(BIN)/server $(BIN)/client

# Ensure bin directory exists
$(BIN):
	mkdir -p $(BIN)

# Build Targets
$(BIN)/server: $(SERVER_SRC) | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built Server"

$(BIN)/client: $(CLIENT_SRC) | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built Client"

$(BIN)/tests: $(TEST_SRC) | $(BIN)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✓ Built Tests"

# Integrated Test Runner (No script needed)
run-tests: $(BIN)/server $(BIN)/tests
	@echo "--- Setting up Environment ---"
	@fuser -k 9000/tcp > /dev/null 2>&1 || true
	@rm -f server.log
	@echo "--- Starting Server (Background) ---"
	@./$(BIN)/server -v 9000 > server.log 2>&1 & echo $$! > server.pid
	@sleep 1
	@echo "--- Running Test Suite ---"
	@if ./$(BIN)/tests; then \
		echo "SUCCESS: All tests passed."; \
		kill `cat server.pid` 2>/dev/null; \
		rm -f server.pid server.log; \
	else \
		echo "FAILURE: Tests failed. Server Log:"; \
		echo "----------------------------------"; \
		cat server.log; \
		echo "----------------------------------"; \
		kill `cat server.pid` 2>/dev/null; \
		rm -f server.pid; \
		exit 1; \
	fi

clean:
	rm -rf $(BIN) server.log server.pid
	@echo "✓ Cleaned"
	
help:
	@echo "IPB Project Makefile"
	@echo "===================="
	@echo "make              Build server and client"
	@echo "make run-tests    Build and run the full test suite (auto-starts server)"
	@echo "make clean        Remove build artifacts and logs"
	@echo "make help         Show this help message"