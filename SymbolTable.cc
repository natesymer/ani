#include "SymbolTable.h"
#include "Types.h"
#include "AST.h"

using namespace std;

ASTClassDecl * SymbolTable::getClass() {
  ASTClassDecl *d = NULL;
  if ((d = dynamic_cast<ASTClassDecl *>(currentDecl))) return d;
  if (parent) return parent->getClass();
  return NULL;
}

ASTDecl * SymbolTable::getCurrentDecl() {
  ASTDecl *d = NULL;
  if (currentDecl) return currentDecl;
  if (parent) return parent->getCurrentDecl();
  return NULL;
}

bool SymbolTable::isGlobal() {
  return !isClassScope();
}

ASTDecl * SymbolTable::declaration_of(string name) {
  ASTDecl * potential = declarations[name];
  ASTClassDecl *cls = getClass();
  if (!potential && cls) potential = cls->lookupMember(name);
  return potential ? potential : (parent ? parent->declaration_of(name) : NULL);
}

void SymbolTable::add(string name, ASTDecl *d) {
  declarations[name] = d;
}

SymbolTable * SymbolTable::getGlobal() {
  if (isGlobal()) return this;
  return parent ? parent->getGlobal() : this;
}

bool SymbolTable::pointsToCurrentDecl(Base *b) {
  ASTDecl *d = getCurrentDecl();
  set<Base *> *sn = new set<Base *>();
  bool v = d ? b->hasPointerTo(d, sn) : false;
  delete sn;
  return v;
}

bool SymbolTable::isClassScope() {
  return getClass() != NULL;
}
