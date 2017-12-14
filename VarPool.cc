#include "VarPool.h"

using namespace std;

void VarPool::add(ASTVarDecl *var) {
  (*positioning)[var->Name] = _size++;
  (*decls)[var->Name] = var;
}

int VarPool::index(string varname) {
  for (VarPool *p = this; p; p = p->parent) {
    auto it = p->positioning->find(varname);
    if (it != p->positioning->end()) {
      int idx = it->second;
      if (p->parent) idx += p->parent->size();
      return idx;
    }
  }
  return -1;
}

int VarPool::size() {
  int parentedSize = _size;
  for (VarPool *p = parent; p; p = p->parent) parentedSize += p->_size;
  return parentedSize;
}

void VarPool::reserveIndex() {
  _size++;
}
