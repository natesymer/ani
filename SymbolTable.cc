#include "SymbolTable.h"
#include "Types.h"
#include "AST.h"

using namespace std;

ASTClassDecl * SymbolTable::getClass() {
  ASTClassDecl *cls = currentClass;
  return cls ? cls : (parent ? parent->getClass() : NULL);
}

bool SymbolTable::isGlobal() {
  return parent == NULL;
}

ASTDecl * SymbolTable::declaration_of(string name) {
  ASTDecl * potential = declarations[name];
  if (!potential && isClassScope()) {
    potential = currentClass->lookupMember(name);
  }
  return potential ? potential : (parent ? parent->declaration_of(name) : NULL);
}

void SymbolTable::add(string name, ASTDecl *d) {
  declarations[name] = d;
}

bool SymbolTable::isClassScope() {
  return currentClass != NULL;
}
