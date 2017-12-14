#if !defined(__SYMBOLTABLE_H__)
#define __SYMBOLTABLE_H__
#include <unordered_map>
#include <iostream>
#include "AST.h"
#include "Types.h"

using namespace std;

class SymbolTable {
  public:
    SymbolTable() : parent(NULL), currentClass(NULL) {};
    SymbolTable(SymbolTable *table) : parent(table), currentClass(NULL) {};
    SymbolTable(SymbolTable *table, ASTClassDecl *cls)
      : parent(table), currentClass(cls) {};
    bool isGlobal();
    ASTDecl *declaration_of(string name);
    void add(string name, ASTDecl *d);
    ASTClassDecl *getClass();
    bool isClassScope();
  private:
    unordered_map<string, ASTDecl *> declarations;
    SymbolTable *parent;
    ASTClassDecl *currentClass;
};

#endif
