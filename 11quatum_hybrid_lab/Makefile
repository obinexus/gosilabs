# GossipLabs C Integration MVP - Build System
# OBINexus Computing - RIFT Ecosystem Integration

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -pthread -O2
LDFLAGS = -pthread
TARGET = gossiplabs_mvp
SOURCES = gossiplabs_mvp.c

# RIFT Integration flags (when available)
RIFT_FLAGS = -lrift --nomeltdown
NLINK_FLAGS = --with-nlink

# Default build
all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "=== Building GossipLabs C Integration MVP ==="
	@echo "=== OBINexus Computing - Services from the Heart ==="
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)
	@echo "=== Build Complete ==="

# RIFT-enabled build (when RIFT toolchain available)
rift: $(SOURCES)
	@echo "=== Building with RIFT Toolchain ==="
	$(CC) $(CFLAGS) $(RIFT_FLAGS) -o $(TARGET)_rift $(SOURCES) $(LDFLAGS)
	@echo "=== RIFT Build Complete ==="

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET)_rift *.o

# Run the MVP demonstration
run: $(TARGET)
	@echo "=== Running GossipLabs MVP ==="
	./$(TARGET)

# Install to system (requires sudo)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	@echo "=== GossipLabs MVP installed to /usr/local/bin/ ==="

# Test build
test: $(TARGET)
	@echo "=== Running Test Suite ==="
	./$(TARGET)
	@echo "=== Tests Complete ==="

# Memory leak check (requires valgrind)
memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# Performance profiling (requires gprof)
profile: CFLAGS += -pg
profile: $(TARGET)
	./$(TARGET)
	gprof $(TARGET) gmon.out > analysis.txt
	@echo "=== Performance analysis saved to analysis.txt ==="

.PHONY: all rift debug clean run install test memcheck profile
