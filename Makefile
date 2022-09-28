all : parser 

parser : syn src/parser.h
	gcc -Isrc out/y.tab.c out/lex.yy.c src/parser_main.c -o ./out/parser

syn : src/syn.lex src/syn.yacc | dir
	yacc -d src/syn.yacc
	lex src/syn.lex
	mv y.tab.* out/
	mv lex.yy.c out/

dir :
	mkdir -p out

clean :
	rm -fr out
