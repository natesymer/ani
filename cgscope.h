#if !defined(__CGSCOPE_H__)
#define __CGSCOPE_H__

#include <ostream>
#include <iostream>
#include <stack>
#include <string>
#include <string.h>
#include "AST.h"
#include "VarPool.h"

using namespace std;

class CGScope {
  public:
    CGScope()
      : output(NULL),
        currentFunction(NULL),
        currentClass(NULL),
        currentInterface(NULL),
        currentBlock(NULL),
        variablePool(new VarPool()),
        breakLabels(new stack<string>()) {};
    CGScope(FILE *out)
      : output(out),
        currentFunction(NULL),
        currentClass(NULL),
        currentInterface(NULL),
        currentBlock(NULL),
        variablePool(new VarPool()),
        breakLabels(new stack<string>()) {};
    CGScope(CGScope *prnt)
      : output(prnt->output),
        currentFunction(prnt->currentFunction),
        currentClass(prnt->currentClass),
        currentInterface(prnt->currentInterface),
        currentBlock(prnt->currentBlock),
        variablePool(new VarPool(prnt->variablePool)),
        breakLabels(new stack<string>(*(prnt->breakLabels))) {};
    ~CGScope() {
      delete breakLabels;
      delete variablePool;
    };

    FILE *output;

    ASTFunctionDecl *currentFunction;
    ASTClassDecl *currentClass;
    TyInterface *currentInterface;
    ASTBlock *currentBlock;

    VarPool *variablePool;
    stack<string> *breakLabels;    
};

CGScope & operator << (CGScope & stream, const char * s) {
  fwrite(s, 1, strlen(s), stream.output);
  return stream;
}

CGScope & operator << (CGScope & stream, string s) {
  fwrite(s.c_str(), 1, s.length(), stream.output);
  return stream;
}

CGScope & operator << (CGScope & stream, double d) {
  stringstream ss;
  ss.precision(6);
  ss.setf(ios::showpoint);
  ss.unsetf(ios::floatfield);
  ss << fixed << d;
  stream << ss.str();
  return stream;
}

CGScope & operator << (CGScope & stream, size_t i) {
  stringstream ss;
  ss << i;
  stream << ss.str();
  return stream;
}

CGScope & operator << (CGScope & stream, int i) {
  stringstream ss;
  ss << i;
  stream << ss.str();
  return stream;
}

#endif
