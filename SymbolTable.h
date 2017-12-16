#if !defined(__SYMBOLTABLE_H__)
#define __SYMBOLTABLE_H__
#include <unordered_map>
#include <iostream>
#include "AST.h"
#include "Types.h"

using namespace std;

class SymbolTable {
  public:
    SymbolTable() : parent(NULL), currentDecl(NULL) {};
    SymbolTable(ASTDecl *decl) : parent(NULL), currentDecl(decl) {};
    SymbolTable(SymbolTable *table) : parent(table), currentDecl(NULL) {};
    SymbolTable(SymbolTable *table, ASTDecl *decl)
      : parent(table), currentDecl(decl) {};
    bool isGlobal();
    ASTDecl *declaration_of(string name);
    void add(string name, ASTDecl *d);
    ASTClassDecl *getClass();
    ASTDecl *getCurrentDecl();
    SymbolTable *getGlobal();
    bool isClassScope();
    bool pointsToCurrentDecl(Base *b);
  private:
    unordered_map<string, ASTDecl *> declarations;
    SymbolTable *parent;
    ASTDecl *currentDecl;
};

#endif
