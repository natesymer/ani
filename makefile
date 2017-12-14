CXX=g++

all: ani

ani: scanner.cc parser.cc $(wildcard *.cc)
	$(CXX) -g -o $@ $^ -std=c++0x

parser.cc parser.h: parser.y
	bison -v --warnings=all,error --defines=parser.h -o parser.cc parser.y

scanner.h scanner.cc: scanner.l
	flex --outfile=scanner.cc --header-file=scanner.h scanner.l

clean:
	/bin/rm -f scanner.{h,cc} parser.{h,cc,output} *.o ani *.class *.jsm *.j
