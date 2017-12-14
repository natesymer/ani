#if !defined(__TYPECHECKER_H__)
#define __TYPECHECKER_H__

#include "AST.h"
#include "Types.h"
#include "SymbolTable.h"

using namespace std;

bool compareTypes(Types *a, Types *b, SymbolTable *scope, bool (*compare)(TyType *, TyType *, SymbolTable *));
void resolveClassHierarchy(ASTClassDecl *cls, SymbolTable *scope);
bool subclassOf(ASTClassDecl *parent, ASTClassDecl *child, SymbolTable *scope);
bool implements(ASTClassDecl *cls, TyInterface *iface, SymbolTable *scope);
ASTDecl * lookupMember(ASTDecl *d, string name);
bool typeEqual(TyType *left, TyType *right, SymbolTable *scope);
bool typeCompat(TyType *left, TyType *right, SymbolTable *scope);
bool hasMember(ASTDecl *decl, string member);
TyType * typeofDecl(ASTDecl *d, SymbolTable *scope);
TyType * typeofExpr(ASTExpr *expr, SymbolTable *scope);
bool load_table(ASTDecls *decls, SymbolTable *scope);
bool typecheck_expr(ASTExpr *expr, SymbolTable *scope);
bool typecheck_block(ASTBlock *block, SymbolTable *scope, bool inBreakable, TyType *returnType = NULL);
bool typecheck_stmt(ASTStmt *stmt, SymbolTable *scope, bool inBreakable = false);
bool typecheck_func(ASTFunctionDecl *func, SymbolTable *scope);
bool typecheck_class(ASTClassDecl *classdecl, SymbolTable *scope);
bool typecheck_decl(ASTDecl *decl, SymbolTable *scope);
bool typecheck(ASTDecls *decls);

#endif
