CXX=g++
cc=g++

# names for executables & source files for parts of the compiler
SCANNER_NAME=scanner
PARSER_NAME=decaf

all: ani

ani: $(SCANNER_NAME).cc $(PARSER_NAME).cc $(wildcard *.cc)
	$(CXX) -g -o $@ $^ -std=c++0x

$(PARSER_NAME).cc $(PARSER_NAME).h: $(PARSER_NAME).y
	bison -v --warnings=all --defines=TokenNumbers.h -o $(PARSER_NAME).cc $(PARSER_NAME).y

$(SCANNER_NAME).h $(SCANNER_NAME).cc: $(SCANNER_NAME).l
	flex --outfile=$(SCANNER_NAME).cc --header-file=$(SCANNER_NAME).h $(SCANNER_NAME).l

clean:
	/bin/rm -f $(SCANNER_NAME).{h,cc} $(PARSER_NAME).{cc,output} TokenNumbers.h *.o ani *.class
