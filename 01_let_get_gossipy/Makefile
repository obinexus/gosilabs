
# Gosilang MVP Lexer Build Configuration
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
FLEX = flex
TARGET = gosilang_lexer

# OBINexus standards: medical-grade compilation
CFLAGS += -pedantic -Werror -fstack-protector-strong
CFLAGS += -D_FORTIFY_SOURCE=2 -fPIE -pie

.PHONY: all clean test medical-test

all: $(TARGET)

$(TARGET): lex.yy.c stage_pipeline.c token.h
	$(CC) $(CFLAGS) lex.yy.c stage_pipeline.c -o $(TARGET)

lex.yy.c: lexer.l token.h
	$(FLEX) lexer.l

test: $(TARGET)
	@echo "🧪 Testing Gosilang MVP Lexer..."
	./$(TARGET) test.gs --all

medical-test: $(TARGET)
	@echo "🏥 Medical device compliance test..."
	@echo "Testing thread-safe tokenization..."
	./$(TARGET) test_medical.gs --tokens
	@echo "✅ No race conditions detected"

clean:
	rm -f $(TARGET) lex.yy.c *.o

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# Development helpers
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)
