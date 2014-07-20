

parser :	main.c syntax.y
	bison -d syntax.y
	flex lexical.l
	gcc -g main.c syntax.tab.c -lfl -ly -o parser

syntax.tab.c:	syntax.y
	bison -d syntax.y

step1:	lexical.l
	flex lexical.l
step2:	syntax.y
	bison -d syntax.y
step3:	lexical.l
	flex lexical.l
step4: main.c syntax.tab.c
	gcc -g main.c syntax.tab.c -lfl -ly -o parser



clean:	
	rm -f lex.yy.c
	rm -f syntax.tab.c
	rm -f syntax.tab.h
	rm -f parser
	rm -f semantic/*.o
	rm -f intercode/*.o
