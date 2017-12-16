#if !defined(__AST_H__)

#define __AST_H__

#include <iostream>
#include <sstream>
#include <list>
#include <cassert>
#include "Token.h"
#include "Base.h"
#include "Types.h"

// This is the base type for Abstract Syntax Trees. All AST's extend this
// class (or a class that extends it)

class AST: public Base {
 public:
  AST() : typechecked(false), typecheckingPassed(false) {};
  virtual string toString() = 0;
  bool typechecked;
  bool typecheckingPassed;
};

// A base class representing declarations

class ASTDecl : public AST {
 public:
  ASTDecl(string name) : AST(), Name(name) {};
  virtual string toString() = 0;
  string Name;
};

// A base class for declarations that can exist in classes

class ASTClassDecl;

class ASTClassableDecl : public ASTDecl {
 public:
  ASTClassableDecl(string name) : ASTDecl(name), Class(NULL) {};
  ASTClassDecl *Class;
};

// A base class for declarations that can contain members

class ASTMemberedDecl : public ASTDecl {
 public:
  ASTMemberedDecl(string name) : ASTDecl(name) {};
  virtual ASTDecl *lookupMember(string name) = 0;
};

// A base class representing statements

class ASTStmt : public AST {
 public:
  ASTStmt() : AST() {};
  virtual string toString() = 0;
};

// A class representing expressions. Objects of this type are empty expressions.

class ASTExpr : public ASTStmt {
 public:
  ASTExpr() : ASTStmt(), ExprType(NULL) {};
  virtual string toString();
  virtual bool isEmpty() { return true; };
  TyType *ExprType;
};

class Types : public list<TyType*>, public Base {
 public:
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seenPtrs);
};

class ASTExprs : public list<ASTExpr*>, public Base {
 public:
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seenPtrs);
};

class ASTDecls : public list<ASTDecl*>, public Base {
 public:
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seenPtrs);
};

class ASTStmts : public list<ASTStmt*>, public Base {
 public:
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seenPtrs);
};

class ASTVarDecl;

// The enumeration for things that can be passed to operators

typedef enum {
  OpAssign = '=',
  OpAdd = '+',
  OpSubtract = '-',
  OpMultiply = '*',
  OpDivide = '/',
  OpModulos = '%',
  OpLess = '<',
  OpLessEqual = 256,
  OpGreater = '>',
  OpGreaterEqual = OpLessEqual + 1,
  OpEqual = OpGreaterEqual + 1,
  OpNotEqual = OpEqual + 1,
  OpLogicalAnd = OpNotEqual + 1,
  OpLogicalOr = OpLogicalAnd + 1,
  OpNot = '!',
  OpIndex = OpLogicalOr + 1
} ASTOp;

// An output operator for displaying various types of ASTs

ostream& operator << (ostream& stream, AST* ast);
ostream& operator << (ostream& stream, ASTDecls* decls);
ostream& operator << (ostream& stream, ASTStmts* stmts);
ostream& operator << (ostream& stream, ASTExprs* exprs);
ostream& operator << (ostream& stream, Types* types);
ostream& operator << (ostream& stream, ASTOp op);

// Unary expressions

class ASTUnaryExpr : public ASTExpr {
 public:
  ASTUnaryExpr(ASTOp op, ASTExpr* left) : ASTExpr(), Op(op), Left(left) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  ASTOp Op;
  ASTExpr* Left;
};

// Binary expressions

class ASTBinaryExpr : public ASTExpr {
 public:
  ASTBinaryExpr(ASTOp op, ASTExpr* left, ASTExpr* right)
    : ASTExpr(), Op(op), Left(left), Right(right) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  ASTOp Op;
  ASTExpr* Left;
  ASTExpr* Right;
};

// Member selection: obj.id

class ASTFieldExpr : public ASTExpr {
 public:
  ASTFieldExpr(ASTExpr* left, string right) : ASTExpr(), Left(left), Right(right) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  ASTExpr* Left;
  string Right;
  ASTDecl *_LeftDecl;
  ASTVarDecl *_RightDecl;
};

// Built-in ReadInteger

class ASTBuiltinReadInteger : public ASTExpr {
 public:
  virtual string toString();
  virtual bool isEmpty() { return false; };
};

// Built-in Print

class ASTBuiltinPrint : public ASTExpr {
 public:
  ASTBuiltinPrint(ASTExprs* expr) : ASTExpr(), Expr(expr) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; }; 
  ASTExprs* Expr;
};

// Built-in ReadLine

class ASTBuiltinReadLine : public ASTExpr {
 public:
  virtual string toString();
  virtual bool isEmpty() { return false; };
};

// Built-in New

class ASTBuiltinNew : public ASTExpr {
 public:
  ASTBuiltinNew(TyType* type) : ASTExpr(), Type(type) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  TyType* Type;
};

// Built-in NewArray

class ASTBuiltinNewArray : public ASTExpr {
 public:
  ASTBuiltinNewArray(ASTExpr* expr, TyType* type) : ASTExpr(), Expr(expr), Type(type) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  ASTExpr* Expr;
  TyType* Type;
};

class ASTDeclExpr : public ASTExpr {
 public:
  ASTDeclExpr(ASTDecl *d) : Decl(d) {};
  ASTDecl *Decl;
};

// Function Call

class ASTFunctionCall : public ASTDeclExpr {
 public:
  ASTFunctionCall(string name, ASTExprs* actuals) : ASTDeclExpr(NULL), Name(name), Actuals(actuals) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  string Name;
  ASTExprs * Actuals;
};

// Method invocation: obj.method(args...)

class ASTMemberFunctionCall : public ASTDeclExpr {
 public:
  ASTMemberFunctionCall(ASTExpr* object, string memberName, ASTExprs* actuals)
    : ASTDeclExpr(NULL), Object(object), MemberName(memberName), Actuals(actuals) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  virtual bool isEmpty() { return false; };
  ASTExpr* Object;
  string MemberName;
  ASTExprs* Actuals;
};

// Integer constant

class ASTIntConst : public ASTExpr {
 public:
  ASTIntConst(int value) : ASTExpr(), Value(value) {};
  virtual string toString();
  virtual bool isEmpty() { return false; };
  int Value;
};

// Double constant

class ASTDoubleConst : public ASTExpr {
 public:
  ASTDoubleConst(double value) : ASTExpr(), Value(value) {};
  virtual string toString();
  virtual bool isEmpty() { return false; };
  double Value;
};

// Bool constant

class ASTBoolConst : public ASTExpr {
 public:
  ASTBoolConst(bool value) : ASTExpr(), Value(value) {};
  virtual string toString();
  virtual bool isEmpty() { return false; };
  bool Value;
};

// String constant

class ASTStringConst : public ASTExpr {
 public:
  ASTStringConst(string value) : ASTExpr(), Value(value) {};
  virtual string toString();
  virtual bool isEmpty() { return false; };
  string Value;
};

// Null constant.

class ASTNullConst : public ASTExpr {
 public:
  virtual string toString();
  virtual bool isEmpty() { return false; };
};

// An object for `this'.

class ASTThis : public ASTExpr {
 public:
  virtual string toString();
  virtual bool isEmpty() { return false; };
};

// A variable reference

class ASTVariable : public ASTDeclExpr {
 public:
  ASTVariable(string name) : ASTDeclExpr(NULL), Name(name) {};
  virtual string toString();
  virtual bool isEmpty() { return false; };
  string Name;
};

// if statement

class ASTIf : public ASTStmt {
 public:
  ASTIf(ASTExpr* cond, ASTStmt* then, ASTStmt* elsePart)
    : ASTStmt(), Cond(cond), Then(then), ElsePart(elsePart) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTExpr* Cond;
  ASTStmt* Then;
  ASTStmt* ElsePart;
};

// while statement

class ASTWhile : public ASTStmt {
 public:
  ASTWhile(ASTExpr* cond, ASTStmt* stmt) : ASTStmt(), Cond(cond), Stmt(stmt) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTExpr* Cond;
  ASTStmt* Stmt;
};

// for statement 

class ASTFor : public ASTStmt {
 public:
  ASTFor(ASTExpr* init, ASTExpr* cond, ASTExpr* incr, ASTStmt* stmt)
    : ASTStmt(), Init(init), Cond(cond), Incr(incr), Stmt(stmt) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTExpr* Init;
  ASTExpr* Cond;
  ASTExpr* Incr;
  ASTStmt* Stmt;
};

// break statement

class ASTBreak : public ASTStmt {
 public:
  ASTBreak() : ASTStmt() {};
  virtual string toString();
 private:
};

// return statement

class ASTReturn : public ASTStmt {
 public:
  ASTReturn(ASTExpr* expr) : ASTStmt(), Expr(expr) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTExpr* Expr;
};

// A block of statements surrounded by { }

class ASTBlock : public ASTStmt {
 public:
  ASTBlock(ASTDecls* decls, ASTStmts* stmts) : ASTStmt(), Decls(decls), Stmts(stmts) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTDecls* Decls;
  ASTStmts* Stmts;
};

// A class declaration and definition

class ASTClassDecl : public ASTMemberedDecl, public TyNamedType {
 public:
  ASTClassDecl(string name, TyType* baseClass, Types* interfaces, ASTDecls* fields)
   : ASTMemberedDecl(name), isGlobal(false), BaseClass(baseClass), Interfaces(interfaces), Fields(fields) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTDecl *lookupMember(string name);
  bool isGlobal; // Whether or not the class is "THE" class.
  TyType* BaseClass;
  Types* Interfaces;
  ASTDecls* Fields;
};

// A variable declaration

class ASTVarDecl : public ASTClassableDecl {
 public:
  ASTVarDecl(TyType* type, string name) : ASTClassableDecl(name), Type(type) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  TyType* Type;
};

// A function declaration and definition

class ASTFunctionDecl : public ASTClassableDecl {
 public:
  ASTFunctionDecl(TyType* returnType, string name, ASTDecls* formals, ASTBlock* block)
   : ASTClassableDecl(name), ReturnType(returnType), Formals(formals), Block(block) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  TyType* ReturnType;
  ASTDecls* Formals;
  ASTBlock* Block;
};

// A function prototype

class TyPrototype : public ASTDecl, public TyType {
 public:
  TyPrototype(string name, TyType* returnType, ASTDecls* formals)
    : ASTDecl(name), ReturnType(returnType), Formals(formals) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  TyType* ReturnType;
  ASTDecls* Formals;
};

// An interface type

class TyInterface : public ASTMemberedDecl, public TyNamedType {
 public:
  TyInterface(string name, Types* prototypes)
    : ASTMemberedDecl(name), Prototypes(prototypes) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  ASTDecl *lookupMember(string name);
  Types* Prototypes;
};

#endif
