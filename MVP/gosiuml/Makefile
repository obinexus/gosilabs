# gosiuml/Makefile
CC = gcc
CFLAGS = -Wall -O2 -I./include -pthread
LDFLAGS = -lm

# Source files
SRCS = src/main.c \
       src/pheno_platform.c \
       src/svg_generator.c \
       src/token_parser.c \
       src/pheno_state_machine.c

OBJS = $(SRCS:.c=.o)
TARGET = gosiuml

# PlantUML for diagram generation
PLANTUML = java -jar plantuml.jar

all: $(TARGET) diagrams

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Generate UML diagrams
diagrams:
	$(PLANTUML) -tsvg docs/state_machine.puml
	xsltproc --output docs/state_machine.svg transform.xsl docs/state_machine.xml

# Generate documentation
docs:
	doxygen Doxyfile
	pandoc -s README.md -o docs/specification.pdf

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f docs/*.svg docs/*.png

test: $(TARGET)
	./$(TARGET) token_file.txt output.svg
	xmllint --schema schema.xsd --noout output.svg

.PHONY: all clean diagrams docs test
