CC = clang
CFLAGS = -Wall
TEST_LEXER = lexer.c test_lexer.c
TEST_PARSER = lexer.c parser.c test_parser.c
USH = lexer.c parser.c ush.c IO.c  func.c main.c

all: test_lexer test_parser ush

test_lexer: $(TEST_LEXER)
	$(CC) $(CFLAGS) -o $@ $^

test_parser: $(TEST_PARSER)
	$(CC) $(CFLAGS) -o $@ $^

ush: $(USH)
	$(CC) $(CFLAGS) -o $@ $^

clean: test_lexer test_parser ush
	rm $^