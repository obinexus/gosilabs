gcc -Wall -Wextra -std=c11 lexer.c pipeline.c -o gosilang
./gosilang test.gs --all
