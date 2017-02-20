all:	avm.c avm.o parser.c scanner.c  SymbolTable.o manage.o stack.o final.o
	gcc -g -o parser parser.c scanner.c SymbolTable.o manage.o stack.o final.o
	rm parser.c scanner.c SymbolTable.o manage.o stack.o 


parser.c: parser.y
	bison --yacc -v --defines -o parser.c parser.y

scanner.c: lexical_an.l
	flex --outfile=scanner.c lexical_an.l

SymbolTable.o: SymbolTable.c
	gcc -g -c -o SymbolTable.o SymbolTable.c

manage.o:manage.c
	gcc -g -c -o manage.o manage.c
stack.o: stack.c
	gcc -g -c -o stack.o stack.c


final.o: final.c
	gcc -g -c -o final.o final.c


clean:
	rm parser *.o parser.output quads.txt
	
avm: avm.c SymbolTable.o stack.o 
	gcc -g -o avm avm.c  stack.o 