#include "typechecker.h"
#include <iostream>
#include <map>
#include <typeinfo>
#include <cstring>
#include "Types.h"
#include "SymbolTable.h"
#include "Base.h"

using namespace std;

template<typename T>
bool is(Base *v) {
  return dynamic_cast<T *>(v) != NULL;
}

// Resolves TyName to actual types in scope.
TyType * resolve(TyType *t, SymbolTable *scope) {
  if (is<TyName>(t)) {
    TyName *nt = dynamic_cast<TyName *>(t);
    string name = nt->Name;
    ASTDecl *res = scope->declaration_of(name);
    TyType *v = dynamic_cast<TyType *>(res);
    if (!v || !is<TyNamedType>(v)) v = new TyUnknown();
    return v;
  }
  return t;
}

bool noRedefinitions(ASTDecls *ds) {
  bool v = true;
  set<string> seenNames;
  for (auto it = ds->begin();
       it != ds->end();
       ++it) {
    string name = (*it)->Name;
    if (seenNames.find(name) != seenNames.end()) {
      v = false;
      break;
    }
    seenNames.insert(name);
  }
  return v;
}

bool sameClass(ASTClassDecl *parent, ASTClassDecl *child) {
  return parent->Name == child->Name;
}

// Returns whether or not child is a subclass of parent
bool subclassOf(ASTClassDecl *parent, ASTClassDecl *child, SymbolTable *scope) {
  bool v = false;
  for (ASTClassDecl *ptr = child;
       ptr;
       ptr = dynamic_cast<ASTClassDecl *>(ptr->BaseClass)) {
    if (sameClass(ptr, parent)) {
      v = true;
      break;
    }
  }
  cout << child->Name << (v ? " is" : " isn't") << " a subclass of " << parent->Name << "." << endl;
  return v;
}

Types *formalsToTypes(ASTDecls *d) {
  Types *ret = new Types();
  if (d) {
    for (auto it = d->begin(); it != d->end(); ++it) {
      ASTVarDecl *vd = dynamic_cast<ASTVarDecl *>(*it);
      if (vd) ret->push_back(vd->Type);
      else ret->push_back(new TyUnknown());
    }
  }
  return ret;
}

bool typesEqual(Types *a, Types *b, SymbolTable *scope) {
  return compareTypes(a, b, scope, &typeEqual);
}

bool typesCompat(Types *a, Types *b, SymbolTable *scope) {
  return compareTypes(a, b, scope, &typeCompat);
}

bool compareTypes(Types *a, Types *b, SymbolTable *scope, bool (*compare)(TyType *, TyType *, SymbolTable *)) {
  for (auto ita = a->begin(), itb = b->begin();
       ita != a->end() && itb != b->end();
       ++ita, ++itb) {
    if (!compare(*ita, *itb, scope)) return false;
  }
  return a->size() == b->size();
}

bool implementsPrototype(ASTClassDecl *cls, TyPrototype *ptype, SymbolTable *scope) {
  bool v = false;
  ASTDecl *member = cls->lookupMember(ptype->Name);
  if (is<ASTFunctionDecl>(member)) {
    ASTFunctionDecl *f = dynamic_cast<ASTFunctionDecl *>(member);
    ptype->ReturnType = resolve(ptype->ReturnType, scope);
    f->ReturnType = resolve(f->ReturnType, scope);
    v = typeCompat(ptype->ReturnType, f->ReturnType, scope)
     && typesCompat(formalsToTypes(f->Formals), formalsToTypes(ptype->Formals), scope);
  }
  cout << "Class " << cls->Name << " " << (v ? "implements" : "doesn't implement") << " " << dynamic_cast<ASTDecl *>(ptype) << endl;
  return v;
}

bool implements(ASTClassDecl *cls, TyInterface *iface, SymbolTable *scope) {
  Types *ifaces = cls->Interfaces;
  bool v = ifaces->size() == 0;

  for (auto it = ifaces->begin(); it != ifaces->end(); ++it) {
    *it = resolve(*it, scope);
    TyType *ty = *it;
    if (is<TyInterface>(ty)) {
      TyInterface *anIface = dynamic_cast<TyInterface *>(ty);
      if (anIface->Name == iface->Name) {
        Types *ps = anIface->Prototypes;
	if (ps->size() == 0) v = true;
	else {
    	  for (auto pit = ps->begin(); pit != ps->end(); ++pit) {
	    v = v || implementsPrototype(cls, dynamic_cast<TyPrototype *>(*pit), scope);
          }
        }
      }
    }
  }
  cout << "Class " << cls->Name << (v ? " implements " : " doesn't implement ") << iface->Name << "." << endl;
  return v;
}

ASTDecl *lookupMember(ASTDecl *d, string name) {
  if (is<ASTClassDecl>(d)) {
    ASTClassDecl *cls = dynamic_cast<ASTClassDecl *>(d);
    return cls->lookupMember(name);
  }
  if (is<TyInterface>(d)) {
    TyInterface *iface = dynamic_cast<TyInterface *>(d);
    return iface->lookupMember(name);
  }
  return NULL;
}

bool typeEqual(TyType *left, TyType *right, SymbolTable *scope) {
  bool v = true;
  if (is<TyNamedType>(left) && is<TyNull>(right)) v = true; // Null is equal to all named types.
  else if (is<ASTClassDecl>(left) && is<ASTClassDecl>(right)) v = sameClass(dynamic_cast<ASTClassDecl *>(left),
                                     					    dynamic_cast<ASTClassDecl *>(right));
  else if (is<TyUnknown>(left) || is<TyUnknown>(right)) v = false;
  else if (is<TyArray>(left) && is<TyArray>(right)) {
    TyArray *l = dynamic_cast<TyArray *>(left);
    TyArray *r = dynamic_cast<TyArray *>(right);
    l->Type = resolve(l->Type, scope);
    r->Type = resolve(r->Type, scope);
    v = typeEqual(l->Type,
	          r->Type, 
		  scope);
  } else v = typeid(*left) == typeid(*right);
  cout << "Types " << (v ? "equal" : "not equal") << ": " << left << " " << right << endl;
  return v;
}

bool typeCompat(TyType *left, TyType *right, SymbolTable *scope) {
  bool v = false;
  if (is<ASTClassDecl>(right)) {
    if (is<ASTClassDecl>(left)) v = sameClass(dynamic_cast<ASTClassDecl *>(left),
					      dynamic_cast<ASTClassDecl *>(right))
				 || subclassOf(dynamic_cast<ASTClassDecl *>(left),
					       dynamic_cast<ASTClassDecl *>(right),
					       scope);
    else if (is<TyInterface>(left)) v = implements(dynamic_cast<ASTClassDecl *>(right),
				         	   dynamic_cast<TyInterface *>(left),
						   scope);
  }
  else if (typeEqual(left, right, scope)) v = true; // First check for type equality.
  cout << "Types " << (v ? "compatible" : "incompatible") << ": " << left << " " << right << endl; 
  return v;
}

bool hasMember(ASTDecl *decl, string member) {
  return lookupMember(decl, member) != NULL;
}

TyType * typeofVarDecl(ASTVarDecl *d, SymbolTable *scope) {
  d->Type = resolve(d->Type, scope);
  return d->Type;
}

TyType * typeofFunctionDecl(ASTFunctionDecl *fd, SymbolTable *scope) {
  fd->ReturnType = resolve(fd->ReturnType, scope);
  return fd->ReturnType;
}

TyType * typeofPrototype(TyPrototype *pt, SymbolTable *scope) {
  pt->ReturnType = resolve(pt->ReturnType, scope);
  return pt->ReturnType;
}

// Returns the expected type of an ASTDecl. Performs no typechecking.
TyType * typeofDecl(ASTDecl *d, SymbolTable *scope) {
  if (is<ASTVarDecl>(d)) return typeofVarDecl(dynamic_cast<ASTVarDecl *>(d), scope);
  if (is<ASTFunctionDecl>(d)) return typeofFunctionDecl(dynamic_cast<ASTFunctionDecl *>(d), scope);
  if (is<ASTClassDecl>(d)) return dynamic_cast<TyType *>(d);
  if (is<TyInterface>(d)) return dynamic_cast<TyType *>(d);
  if (is<TyPrototype>(d)) return typeofPrototype(dynamic_cast<TyPrototype *>(d), scope);
  return new TyUnknown();
}

// Returns the expected type of an ASTExpr. Performs no typechecking.
TyType * typeofExpr(ASTExpr *expr, SymbolTable *scope) {
  if (expr && expr->ExprType != NULL) {
    cout << "Expression of type " << expr->ExprType << "; " << expr << endl;
    return expr->ExprType;
  }

  TyType *v = NULL;
  if (expr->isEmpty()) v = new TyVoid();
  else if (is<ASTBoolConst>(expr)) v = new TyBool();
  else if (is<ASTStringConst>(expr)) v = new TyString();
  else if (is<ASTDoubleConst>(expr)) v = new TyDouble();
  else if (is<ASTIntConst>(expr)) v = new TyInt();
  else if (is<ASTNullConst>(expr)) v = new TyNull();
  else if (is<ASTVariable>(expr)) {
    string name = dynamic_cast<ASTVariable *>(expr)->Name;
    ASTDecl *d = scope->declaration_of(name);
    if (!d && scope->getClass()) d = scope->getClass()->lookupMember(name);
    v = typeofDecl(d, scope);
  }
  else if (is<ASTFunctionCall>(expr)) {
    string name = dynamic_cast<ASTFunctionCall *>(expr)->Name;
    ASTDecl *d = scope->declaration_of(name);
    if (!d && scope->getClass()) d = scope->getClass()->lookupMember(name);
    v = typeofDecl(d, scope);
  }
  else if (is<ASTBuiltinReadLine>(expr)) v = new TyString();
  else if (is<ASTBuiltinReadInteger>(expr)) v = new TyInt();
  else if (is<ASTBuiltinPrint>(expr)) v = new TyVoid();
  else if (is<ASTBuiltinNew>(expr)) {
    ASTBuiltinNew *n = dynamic_cast<ASTBuiltinNew *>(expr);
    v = resolve(n->Type, scope);
    n->Type = v;
  }
  else if (is<ASTBuiltinNewArray>(expr)) {
    ASTBuiltinNewArray *na = dynamic_cast<ASTBuiltinNewArray *>(expr);
    na->Type = resolve(na->Type, scope);
    v = new TyArray(na->Type);
  }
  else if (is<ASTMemberFunctionCall>(expr)) {
    ASTMemberFunctionCall *funcall = dynamic_cast<ASTMemberFunctionCall *>(expr);
    TyType *onType = typeofExpr(funcall->Object, scope);
    
    if ((is<TyArray>(onType) || is<TyString>(onType))
	&& funcall->MemberName == "length"
	&& funcall->Actuals->size() == 0) v = new TyInt();
    else if (is<ASTDecl>(onType)) {
      ASTDecl *onDecl = dynamic_cast<ASTDecl *>(onType);
      ASTDecl *hopefullyFunc = lookupMember(onDecl, funcall->MemberName);
      if (is<ASTFunctionDecl>(hopefullyFunc) || is<TyPrototype>(hopefullyFunc)) {
	v = typeofDecl(hopefullyFunc, scope);
      }
    }
  }
  else if (is<ASTFieldExpr>(expr)) {
    ASTFieldExpr *fielde = dynamic_cast<ASTFieldExpr *>(expr);
    TyType *onType = typeofExpr(fielde->Left, scope);
    if (is<ASTDecl>(onType)) {
      ASTDecl *onDecl = dynamic_cast<ASTDecl *>(onType);
      ASTDecl *hopefullyVar = lookupMember(onDecl, fielde->Right);
      if (is<ASTVarDecl>(hopefullyVar)) v = typeofDecl(hopefullyVar, scope);
    }
  }
  else if (is<ASTThis>(expr) && scope->getClass()) v = scope->getClass();
  else if (is<ASTUnaryExpr>(expr)) {
    ASTUnaryExpr *u = dynamic_cast<ASTUnaryExpr *>(expr);
    v = typeofExpr(u->Left, scope);
  }
  else if (is<ASTBinaryExpr>(expr)) {
    ASTBinaryExpr *b = dynamic_cast<ASTBinaryExpr *>(expr);
    switch (b->Op) {
      case OpAssign:
	v = typeofExpr(b->Left, scope);
	break;
      case OpAdd:
      case OpSubtract:
      case OpMultiply:
      case OpDivide:
      case OpModulos:
	v = typeofExpr(b->Left, scope);
	break;
      case OpLess:
      case OpLessEqual:
      case OpGreater:
      case OpGreaterEqual:
      case OpEqual:
      case OpNotEqual:
      case OpLogicalAnd:
      case OpLogicalOr:
	v = new TyBool();
	break;
      case OpIndex: {
	TyType *hopefullyArray = typeofExpr(b->Left, scope);
	if (is<TyArray>(hopefullyArray)) {
	  v = dynamic_cast<TyArray *>(hopefullyArray)->Type;
        }
      }
      default: {}
    }
  }

  if (v) {
    expr->ExprType = resolve(v, scope);
    cout << "Expression of type " << v << "; " << expr << endl;
    return v;
  }

  cout << "Unknown type for: " << expr << endl;
  return new TyUnknown();
}

/* Runs through all of the provided declarations and adds them to 
   the provided symbol table. */
bool load_table(ASTDecls *decls, SymbolTable *scope) {
  bool v = true;
  set<string> names;
  for (auto it = decls->begin(); it != decls->end(); ++it) {
    ASTDecl *d = *it;
    if (names.count(d->Name) == 0) {
      scope->add(d->Name, d);
      names.insert(d->Name);
    } else {
      v = false;
      break;
    }
  }
  cout << (v ? "Successfully loaded scope" : "Failed to load scope") << endl;
  return v;
}

bool typecheck_formals(ASTDecls *formals, SymbolTable *scope) {
  bool v = true;

  set<string> *names = new set<string>();
  for (auto it = formals->begin(); it != formals->end(); ++it) {
    ASTVarDecl *d = dynamic_cast<ASTVarDecl *>(*it);
    if (d) {
      d->Type = resolve(d->Type, scope);
      if (!is<TyUnknown>(d->Type) && names->count(d->Name) == 0) {
	names->insert(d->Name);
        continue;
      }
    }
    v = false;
    break;
  }
  delete names;
  
  cout << "Formals " << (v ? "passed" : "failed") << ":" << formals << endl;
  return v;
}

bool typecheck_args(ASTDecls *formals, ASTExprs *args, SymbolTable *scope) {
  if (!args) args = new ASTExprs();
  if (!formals) formals = new ASTDecls();

  bool v = true;
  auto ita = args->begin();
  auto itb = formals->begin();
  while (ita != args->end() && itb != formals->end()) {
    ASTExpr *a = *ita;
    if (!typecheck_expr(a, scope)) {
      v = false;
      break;
    }
    ASTDecl *b = *itb;
    if (!typecheck_decl(b, scope)) {
      v = false;
      break;
    }
    if (!typeCompat(typeofDecl(b, scope), typeofExpr(a, scope), scope)) {
      v = false;
      break;
    }
    ++ita;
    ++itb;
  }

  v = v && args->size() == formals->size();

  cout << "Function args " << (v ? "pass." : "fail.") << endl;
  return v;
}

bool typecheck_expr(ASTExpr *expr, SymbolTable *scope) {
  typeofExpr(expr, scope); // ensure all expressions have their ExprType set.
                           // this is cached and returned the next time this information is requested.
  bool v = false;
  if (expr->isEmpty()) v = true;
  else if (is<ASTBoolConst>(expr)) v = true;
  else if (is<ASTStringConst>(expr)) v = true;
  else if (is<ASTDoubleConst>(expr)) v = true;
  else if (is<ASTIntConst>(expr)) v = true;
  else if (is<ASTNullConst>(expr)) v = true;
  else if (is<ASTBuiltinReadLine>(expr)) v = true;
  else if (is<ASTBuiltinReadInteger>(expr)) v = true;
  else if (is<ASTBuiltinNew>(expr)) {
    ASTBuiltinNew *n = dynamic_cast<ASTBuiltinNew *>(expr);
    n->Type = resolve(n->Type, scope);
    v = is<ASTClassDecl>(n->Type);
  }
  else if (is<ASTVariable>(expr)) {
    ASTVariable *var = dynamic_cast<ASTVariable *>(expr);
    ASTDecl *d = scope->declaration_of(var->Name);
    if (!d && scope->getClass()) d = scope->getClass()->lookupMember(var->Name);
    var->Decl = d;
    v = d && typecheck_decl(d, scope);
  }
  else if (is<ASTFunctionCall>(expr)) {
    ASTFunctionCall *fc = dynamic_cast<ASTFunctionCall *>(expr);
    ASTDecl *d = scope->declaration_of(fc->Name);
    if (!d && scope->getClass()) d = scope->getClass()->lookupMember(fc->Name);
    if (d && is<ASTFunctionDecl>(d)) {
      ASTFunctionDecl *fd = dynamic_cast<ASTFunctionDecl *>(d);
      fc->Decl = fd;
      v = typecheck_args(fd->Formals, fc->Actuals, scope);
    }
  }
  else if (is<ASTBuiltinPrint>(expr)) {
    v = true;
    ASTBuiltinPrint *p = dynamic_cast<ASTBuiltinPrint *>(expr);
    ASTExprs *es = p->Expr;
    for (auto it = es->begin(); it != es->end(); ++it) {
      ASTExpr *pe = *it;
      if (!typecheck_expr(pe, scope)) {
	v = false;
	break;
      }
      else {
	TyType *t = typeofExpr(pe, scope);
	if (!(typeEqual(t, new TyString(), scope)
	      || typeEqual(t, new TyInt(), scope)
	      || typeEqual(t, new TyBool(), scope))) {
	  v = false;
	  break;
	}
      }
    }
  }
  else if (is<ASTBuiltinNewArray>(expr)) {
    ASTBuiltinNewArray *na = dynamic_cast<ASTBuiltinNewArray *>(expr);
    na->Type = resolve(na->Type, scope);
    v = typecheck_expr(na->Expr, scope) && is<TyInt>(typeofExpr(na->Expr, scope)) && !is<TyUnknown>(na->Type);
  }
  else if (is<ASTMemberFunctionCall>(expr)) {
    ASTMemberFunctionCall *funcall = dynamic_cast<ASTMemberFunctionCall *>(expr);
    if (typecheck_expr(funcall->Object, scope)) {
      TyType *onType = typeofExpr(funcall->Object, scope);
      if (is<TyArray>(onType) || is<TyString>(onType)) {
	v = funcall->MemberName == "length"
	  && funcall->Actuals->size() == 0;
      } else if (is<ASTDecl>(onType)) {
        ASTDecl *hopefullyFunc = lookupMember(dynamic_cast<ASTDecl *>(onType), funcall->MemberName);
        ASTDecls *formals = NULL;
        if (is<ASTFunctionDecl>(hopefullyFunc)) formals = dynamic_cast<ASTFunctionDecl *>(hopefullyFunc)->Formals;
        else if (is<TyPrototype>(hopefullyFunc)) formals = dynamic_cast<TyPrototype *>(hopefullyFunc)->Formals;

        if (formals) {
	  ASTClassDecl *cd = dynamic_cast<ASTClassDecl *>(onType);
	  SymbolTable *subscope = cd ? new SymbolTable(scope, cd) : scope;
	  funcall->Decl = hopefullyFunc;
	  v = typecheck_args(formals, funcall->Actuals, subscope);
	}
      }
    }
  }
  else if (is<ASTFieldExpr>(expr)) {
    ASTFieldExpr *fielde = dynamic_cast<ASTFieldExpr *>(expr);
    if (typecheck_expr(fielde->Left, scope)) {
      TyType *onType = typeofExpr(fielde->Left, scope);
      if (is<ASTDecl>(onType) && scope->getClass() && typeCompat(onType, scope->getClass(), scope)) {
	fielde->_LeftDecl = dynamic_cast<ASTDecl *>(onType);
        ASTDecl *hopefullyVar = lookupMember(fielde->_LeftDecl, fielde->Right);
        if (is<ASTVarDecl>(hopefullyVar)) {
	  fielde->_RightDecl = dynamic_cast<ASTVarDecl *>(hopefullyVar);
	  v = typecheck_decl(hopefullyVar, scope);
	}
      }
    }
  }
  else if (is<ASTThis>(expr)) v = scope->getClass() != NULL;
  else if (is<ASTUnaryExpr>(expr)) {
    ASTUnaryExpr *u = dynamic_cast<ASTUnaryExpr *>(expr);
    if (typecheck_expr(u->Left, scope)) {
      TyType *t = typeofExpr(u->Left, scope);
      switch (u->Op) {
        case OpNot:
	  v = typeEqual(t, new TyBool(), scope);
	  break;
        case OpSubtract:
  	  v = typeEqual(t, new TyInt(), scope) || typeEqual(t, new TyDouble(), scope);
	  break;
        default: {}
      }
    }
  }
  else if (is<ASTBinaryExpr>(expr)) {
    ASTBinaryExpr *b = dynamic_cast<ASTBinaryExpr *>(expr);
    if (typecheck_expr(b->Left, scope) && typecheck_expr(b->Right, scope)) {
      TyType *l = typeofExpr(b->Left, scope);
      TyType *r = typeofExpr(b->Right, scope);
      switch (b->Op) {
        case OpAssign:
          v = typeCompat(l, r, scope);
	  break;
        case OpAdd:
        case OpSubtract:
        case OpMultiply:
        case OpDivide:
        case OpModulos:
        case OpLess:
        case OpLessEqual:
        case OpGreater:
        case OpGreaterEqual:
          v = (typeEqual(l, new TyInt(), scope) && typeEqual(r, new TyInt(), scope))
           || (typeEqual(l, new TyDouble(), scope) && typeEqual(r, new TyDouble(), scope));
	  break;
        case OpEqual:
        case OpNotEqual:
          v = typeCompat(l, r, scope) || typeCompat(r, l, scope);
	  break;
        case OpLogicalAnd:
        case OpLogicalOr:
	  v = typeEqual(l, new TyBool(), scope)
 	   && typeEqual(r, new TyBool(), scope);
	  break;
        case OpIndex:
          v = is<TyArray>(l)
 	   && typeEqual(r, new TyInt(), scope);
	  break;
        default: {}
      }
    }
  }
  cout << "Expression " << (v ? "passes" : "fails") << ": " << expr << endl;
  return v;
}

bool typecheck_block(ASTBlock *block, SymbolTable *scope, bool inBreakable, TyType *returnType) {
  ASTDecls *bdecls = block->Decls;
  ASTStmts *stmts = block->Stmts;

  bool v = true;

  for (auto it = bdecls->begin(); it != bdecls->end(); ++it) {
    v = typecheck_decl(*it, scope);
    if (!v) break;
  }

  if (v) {
    // Create a new scope & load the decls into it.
    SymbolTable *blockscope = new SymbolTable(scope);
    if (load_table(bdecls, blockscope)) {
      // Typecheck each statement.
      for (auto it = stmts->begin(); it != stmts->end(); ++it) {
        ASTStmt *s = *it;
	v = typecheck_stmt(s, blockscope, inBreakable);
	if (!v) break;

        if (returnType && is<ASTReturn>(s)) {
          ASTReturn *r = dynamic_cast<ASTReturn *>(s);
	  v = typeCompat(returnType, typeofExpr(r->Expr, blockscope), blockscope);
	  if (!v) break;
        }
      }
    }
    
    delete blockscope;
  }

  v = v && noRedefinitions(bdecls);

  cout << "Block " << (v ? "passes: " : "fails: ") << block << endl;
  return v;
}

bool typecheck_stmt(ASTStmt *stmt, SymbolTable *scope, bool inBreakable) {
  bool v = false;
  if (!stmt) v = true;
  else if (is<ASTBreak>(stmt) && inBreakable) v = true;
  else if (is<ASTIf>(stmt)) {
    ASTIf *casted = dynamic_cast<ASTIf *>(stmt);
    v = typeEqual(typeofExpr(casted->Cond, scope), new TyBool(), scope)
     && typecheck_expr(casted->Cond, scope)
     && typecheck_stmt(casted->Then, scope, inBreakable)
     && typecheck_stmt(casted->ElsePart, scope, inBreakable);
  }
  else if (is<ASTWhile>(stmt)) {
    ASTWhile *casted = dynamic_cast<ASTWhile *>(stmt);
    v = typeEqual(typeofExpr(casted->Cond, scope), new TyBool(), scope)
     && typecheck_expr(casted->Cond, scope)
     && typecheck_stmt(casted->Stmt, scope, true);
  }
  else if (is<ASTFor>(stmt)) {
    ASTFor *casted = dynamic_cast<ASTFor *>(stmt);
    v = typeEqual(typeofExpr(casted->Cond, scope), new TyBool(), scope)
     && typecheck_expr(casted->Init, scope)
     && typecheck_expr(casted->Cond, scope)
     && typecheck_expr(casted->Incr, scope)
     && typecheck_stmt(casted->Stmt, scope, true);
  }
  else if (is<ASTReturn>(stmt)) v = typecheck_expr(dynamic_cast<ASTReturn *>(stmt)->Expr, scope);
  else if (is<ASTBlock>(stmt)) v = typecheck_block(dynamic_cast<ASTBlock *>(stmt), scope, inBreakable);
  else if (is<ASTExpr>(stmt)) v = typecheck_expr(dynamic_cast<ASTExpr *>(stmt), scope);

  cout << "Statement " << (inBreakable ? "(in breakable) " : "")  << (v ? "passes" : "fails") << ": " << stmt << endl;
  return v;
}

bool typecheck_func(ASTFunctionDecl *func, SymbolTable *scope) {
  func->ReturnType = resolve(func->ReturnType, scope);
  func->Class = scope->getClass();

  bool v = false;
  if (typecheck_formals(func->Formals, scope)) {
    SymbolTable *formalsScope = new SymbolTable(scope);
    if (load_table(func->Formals, formalsScope)) {
      v = !is<TyUnknown>(func->ReturnType)
       && typecheck_formals(func->Formals, scope)
       && typecheck_block(func->Block, formalsScope, false, func->ReturnType);
    }
  
    delete formalsScope;
  }

  if (v && scope->getClass()) {
    ASTClassDecl *supercls = dynamic_cast<ASTClassDecl *>(scope->getClass()->BaseClass);
    if (supercls) {
      ASTDecl *member = supercls->lookupMember(func->Name);
      if (member && is<ASTFunctionDecl>(member)) {
        ASTFunctionDecl *memFunc = dynamic_cast<ASTFunctionDecl *>(member);
        v = typesEqual(formalsToTypes(memFunc->Formals), formalsToTypes(func->Formals), scope);
      }
    }
  }

  func->Class = scope->getClass();
  cout << "Function " << func->Name << (v ? " passes." : " fails.") << endl;
  return v;
}

bool typecheck_class(ASTClassDecl *classdecl, SymbolTable *scope) {
  bool v = true;

  // Resolve BaseClasses
  // Ensure no circularity.
  ASTClassDecl *cd = classdecl;
  while (cd->BaseClass) {
    if (is<TyArray>(cd->BaseClass)) {
      v = false;
      break;
    }
    TyType *ty = resolve(cd->BaseClass, scope);
    cd->BaseClass = ty;
    ASTClassDecl *resolved = dynamic_cast<ASTClassDecl *>(ty);
    if (resolved && !sameClass(resolved, classdecl)) {
      cd = resolved;
    } else {
      v = false;
      break;
    }
  }
  
  if (v) {
    // Typecheck class body
    ASTDecls *cbody = classdecl->Fields;
    SymbolTable *classScope = new SymbolTable(scope, classdecl);
    if (load_table(cbody, classScope)) {
      for (auto it = cbody->begin(); it != cbody->end(); ++it) {
        if (!typecheck_decl(*it, classScope)) {
          v = false;
          break;
        } else if (is<ASTClassDecl>(classdecl->BaseClass)) {
	  ASTClassDecl *bc = dynamic_cast<ASTClassDecl *>(classdecl->BaseClass);
	  ASTDecl *d = *it;
	  ASTDecl *baseClassD = bc->lookupMember(d->Name);

	  if (is<ASTFunctionDecl>(baseClassD) && is<ASTFunctionDecl>(d)) {
	    ASTFunctionDecl *fd = dynamic_cast<ASTFunctionDecl *>(d);
	    ASTFunctionDecl *bcfd = dynamic_cast<ASTFunctionDecl *>(baseClassD);
	    v = typeEqual(bcfd->ReturnType, fd->ReturnType, scope)
	     && typesEqual(formalsToTypes(bcfd->Formals), formalsToTypes(fd->Formals), scope);
	    if (!v) break;
	  } else if (baseClassD) {
	    v = false;
	    break;
          }
        }
      }
    } else v = false;
    delete classScope;

    if (v) {
      // Check & resolve interfaces.
      Types *ifaces = classdecl->Interfaces;
      for (auto it = ifaces->begin(); it != ifaces->end(); ++it) {
        *it = resolve(*it, scope);
        if (is<TyInterface>(*it)) {
	  ASTClassDecl *d = classdecl;
	  TyInterface *iface = dynamic_cast<TyInterface *>(*it);
	  v = implements(d, iface, scope);
	  if (!v) break;
        } else {
          v = false;
          break;
        }
      }
      
      v = v && noRedefinitions(classdecl->Fields);
    }
  }

  cout << "Class " << classdecl->Name << (v ? " passes." : " fails.") << endl;
  return v;
}

bool typecheck_interface(TyInterface *iface, SymbolTable *scope) {
  Types *ps = iface->Prototypes;
  bool v = true;

  for (auto it = ps->begin(); it != ps->end(); ++it) {
    if (is<TyPrototype>(*it)) {
      TyPrototype *pt = dynamic_cast<TyPrototype *>(*it);
      if (!typecheck_decl(pt, scope)) {
	v = false;
	break;
      }
    } else {
      v = false;
      break;
    }
  }

  cout << "Interface " << iface->Name << (v ? " passes." : " fails.") << endl;
  return v;
}

bool typecheck_prototype(TyPrototype *pt, SymbolTable *scope) {
  bool v = true;
  pt->ReturnType = resolve(pt->ReturnType, scope);
  if (is<TyUnknown>(pt->ReturnType)) v = false;
  else {
    for (auto it = pt->Formals->begin(); it != pt->Formals->end(); ++it) {
      if (!typecheck_decl(*it, scope)) {
	v = false;
	break;
      }
    }
  }
  cout << "Prototype " << pt->Name << (v ? " passes." : " fails.") << endl;
  return v;
}

bool typecheck_var_decl(ASTVarDecl *vd, SymbolTable *scope) {
  vd->Class = scope->getClass();
  vd->Type = resolve(vd->Type, scope);

  bool v = !(is<TyUnknown>(vd->Type) || is<TyVoid>(vd->Type));
  if (v && scope->isClassScope()) {
    ASTClassDecl *cls = scope->getClass();
    if (cls->BaseClass && is<ASTClassDecl>(cls->BaseClass)) {
      ASTClassDecl *supercls = dynamic_cast<ASTClassDecl *>(cls->BaseClass);
      v = !hasMember(supercls, vd->Name);
    }
  }
  cout << "VariableDeclaration " << vd->Name << (v ? " passes." : " fails.")  << endl;
  return v;
}

bool typecheck_decl(ASTDecl *decl, SymbolTable *scope) {
  if (is<ASTVarDecl>(decl)) return typecheck_var_decl(dynamic_cast<ASTVarDecl *>(decl), scope);
  if (is<ASTFunctionDecl>(decl)) return typecheck_func(dynamic_cast<ASTFunctionDecl *>(decl), scope);
  if (is<TyPrototype>(decl)) return typecheck_prototype(dynamic_cast<TyPrototype *>(decl), scope);
  if (scope->isGlobal()) {
    if (is<ASTClassDecl>(decl)) return typecheck_class(dynamic_cast<ASTClassDecl *>(decl), scope);
    if (is<TyInterface>(decl)) return typecheck_interface(dynamic_cast<TyInterface *>(decl), scope);
  }
  cout << "Unknown/Unexpected Declaration: " << decl->Name << endl;
  return false;
}

// Entry point of typechecker.
bool typecheck(ASTDecls *decls) {
  SymbolTable *global_scope = new SymbolTable();
  
  bool v = false;
  if (load_table(decls, global_scope)) {
    v = true;
    for (auto it = decls->begin(); it != decls->end(); ++it) {
      if (!typecheck_decl(*it, global_scope)) {
        v = false;
        break;
      }
    }
  }
  cout << "typechecking " << (v ? "succeeded." : "failed.") << endl;
  return v;
}
