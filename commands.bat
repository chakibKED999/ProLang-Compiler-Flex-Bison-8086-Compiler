bison -d syn.y
flex lexical.l
gcc lex.yy.c syn.tab.c quad.c optim.c codegen.c -lfl -ly -o TP.exe
