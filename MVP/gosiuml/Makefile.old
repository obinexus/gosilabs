# gosiuml/Makefile
CC = gcc
CFLAGS = -Wall -O2 -I./include -pthread
LDFLAGS = -lm

# Source files
SRCS = src/main.c \
       src/token_parser.c \
       src/svg_generator.c \
       src/pheno_state_machine.c

OBJS = $(SRCS:.c=.o)
TARGET = gosiuml

# PlantUML for diagram generation (optional)
PLANTUML = java -jar plantuml.jar

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Generate UML diagrams (if plantuml is available)
diagrams:
	@if [ -f images/gosiuml_state_diagram.plantuml ]; then \
		$(PLANTUML) -tsvg images/gosiuml_state_diagram.plantuml; \
	fi

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.svg

test: $(TARGET)
	@echo "Creating test token file..."
	@echo "# Test token file" > token_file.txt
	@echo "1 0x01 NODE_IDENTITY" >> token_file.txt
	@echo "2 0x02 NODE_STATE" >> token_file.txt
	@echo "3 0x04 CLUSTER_TOPOLOGY" >> token_file.txt
	@echo "4 0x06 FRAME_REFERENCE" >> token_file.txt
	@echo "5 0x03 NODE_DEGRADATION" >> token_file.txt
	./$(TARGET) token_file.txt output.svg
	@echo "Generated output.svg - open in browser to view"

run: $(TARGET)
	./$(TARGET) config/token_file.txt gosiuml.svg

.PHONY: all clean diagrams test run
