#include <list>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace std;

#include "AST.h"
#include "parser.h"

ASTDecls* parser(FILE* input);
bool typecheck(ASTDecls* tree);
list<string> codegen(string dir, string fileName, ASTDecls* tree);

void usage(char *programName) {
  cerr << "Usage: " << programName <<
    " -s -p -t -c [[input-filename] output-filename]" << endl;
  exit(1);
}

extern FILE* yyin;

int yylex();

class TokIDMarker : public Token {
 public:
  TokIDMarker() : Token('@', string("I'm a TYPEID marker!"), NULL, 0, 0) {
  }
  virtual string toString() {
    return "I'm a TYPEID marker!";
  }
};

static list<Token*>* getTokens() {
  list<Token*>* tokens = new list<Token*>();
  int tokNum;

  while ((tokNum = yylex()) != 0) {
    if (tokNum == TokTYPEID)
      tokens->push_back(new TokIDMarker());
    tokens->push_back(yylval.token);
  }
  return tokens;
}

static void scanner(FILE* input) {
  yyin = input;
  list<Token*>* tokens = getTokens();

  bool marker = false;
  for (list<Token*>::iterator it = tokens->begin(); it != tokens->end(); it++) {
    if ((*it)->toString() == "I'm a TYPEID marker!") {
      marker = true;
    }
    else {
      if (marker) {
	marker = false;
	cout << "Token: TYPE" << **it << endl;
      }
      else
	cout << "Token: " << **it << endl;
    }
  }
}


void processCommandLine(int argc, char* argv[], bool& scan, bool& parse, 
                        bool& typecheck, bool& code, FILE*& input,
			string& outputFileName) {
  scan = parse = typecheck = false;
  input = stdin;
  outputFileName = "stdout";
  int c = 'x';

  while ((c = getopt(argc, argv, "sptc")) != -1) {
    switch (c) {
    case 's':
      scan = true;
      parse = false;
      typecheck = false;
      code = false;
      break;
    case 'p':
      scan = false;
      parse = true;
      typecheck = false;
      code = false;
      break;
    case 't':
      scan = false;
      parse = false;
      typecheck = true;
      code = false;
      break;
    case 'c':
      scan = false;
      parse = false;
      typecheck = false;
      code = true;
      break;
    case '?':
      usage(argv[0]);

    default:
      usage(argv[0]);
    }
  }

  
  int args = argc - optind;
  if (args > 2)
    usage(argv[0]);

  if (args > 0) {
    input = fopen(argv[optind], "r");
    if (!input) {
      cerr << "Error opening file for input: " << argv[optind] << endl;
      exit(1);
    }
  }
  if (args > 1) {
    outputFileName = argv[optind + 1];
  }
}
    
int main(int argc, char* argv[]) {
  ASTDecls* tree;
  bool scan = true;
  bool parse = true;
  bool type = true;
  bool code = true;

  string outputFileName = "";

  processCommandLine(argc, argv, scan, parse, type, code, yyin, outputFileName);

  if (scan) {
    scanner(yyin);
  }
  if (parse || type || code) {
    tree = parser(yyin);
    if (parse) {
      cout << tree << endl;
      cout << "Valid program!" << endl;
    }
  }  

  if (type || code)
    if (!typecheck(tree))
      return 1;

  list<string> fileNames;
  
  if (code) {
    // TODO: improve this interface:
    //       1. Take input file name and optional output directory for .class files
    //       2. Maybe just compile classes
    // Find the directory

    int slash = outputFileName.rfind('/');
    string dir;

    if (slash == -1) {
      dir = "./";
    }
    else {
      dir = outputFileName.substr(0, slash + 1);
      outputFileName = outputFileName.substr(slash + 1);
    }

    // Get rid of the extension, if there is one

    int dot = outputFileName.find('.');
    string extension;
    if (dot == -1) {
      extension = "";
    }
    else {
      extension = outputFileName.substr(dot);
      outputFileName = outputFileName.substr(0, dot);
    }

    cout << "Generating to: " << outputFileName << endl << dir << endl;

    fileNames = codegen(dir, outputFileName, tree);

    for (auto it = fileNames.begin(); it != fileNames.end(); it++) {
      string name = *it;
      cerr << "Assembling file " << name << endl;
      system(("jasmin " + name).c_str());
    }
  }
  return 0;
}
