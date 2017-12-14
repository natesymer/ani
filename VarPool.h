#if !defined(__VARPOOL_H__)
#define __VARPOOL_H__

#include <map>
#include "AST.h"
#include "Types.h"

using namespace std;

class VarPool {
  public:
    VarPool()
      : parent(NULL), _size(0),
        decls(new map<string, ASTVarDecl *>()),
        positioning(new map<string, int>()) {};
    VarPool(VarPool *prnt)
      : parent(prnt), _size(0),
        decls(new map<string, ASTVarDecl *>()),
        positioning(new map<string, int>()) {};
    ~VarPool() {
      delete decls;
      delete positioning;
    };
    void add(ASTVarDecl *var);
    int index(string varname);
    int size();
    void reserveIndex();
  private:
    VarPool *parent;
    int _size;
    map<string, ASTVarDecl *> *decls;
    map<string, int> *positioning;			    
};

#endif
