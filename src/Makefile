CC = clang
CXX = clang++

LLVMFLAGS = `llvm-config --cppflags --ldflags --libs core jit native`
CXXFLAGS = -g $(LLVMFLAGS) -fcxx-exceptions -frtti
LDFLAGS = -ll

YACC = yacc -d
LEX = lex

all: main.o parse.o lex.yy.o token.o ast.o code_generator.o ast_visitor.o
	$(CXX) $(CXXFLAGS) main.o parse.o lex.yy.o token.o ast.o code_generator.o ast_visitor.o $(LDFLAGS) -o stone

stone: all	

main.o: parse.hh

parse.hh: parse.cc
parse.cc: parse.y
	$(YACC) parse.y -o parse.cc

lex.yy.cc: lex.l
	$(LEX) -o lex.yy.cc lex.l

lex.yy.o: parse.hh

sample: stone 
	./stone ../samples/sample.stone

clean:
	rm -f stone lex.yy.cc parse.cc parse.hh y.tab.c y.tab.h *.o
