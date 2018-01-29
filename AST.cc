#include <sstream>
#include "AST.h"

/*
  Typechecker
 */

bool ASTDecl::sameAs(ASTDecl *d) {
  return this->Name == d->Name;
}

bool ASTClassDecl::subclassOf(ASTClassDecl *parent) {
  bool v = false;
  for (ASTClassDecl *ptr = this;
       ptr;
       ptr = dynamic_cast<ASTClassDecl *>(ptr->BaseClass)) {
    if (ptr->sameAs(parent)) {
      v = true;
      break;
    }
  }
  cout << this->Name << (v ? " is" : " isn't") << " a subclass of " << parent->Name << "." << endl;
  return v;
}

ASTDecl * TyInterface::lookupMember(string name) {
  for (auto it = Prototypes->begin();
       it != Prototypes->end();
       ++it) {
    TyType *t = *it;
    ASTDecl *adecl = dynamic_cast<ASTDecl *>(t);
    if (adecl) {
      if (adecl->Name == name) return adecl;
    }
  }
  return NULL;
}

ASTDecl * ASTClassDecl::lookupMember(string name) {
  for (auto it = Fields->begin();
       it != Fields->end();
       ++it) {
    ASTDecl *dl = *it;
    if (dl->Name == name) return dl;
  }
  if (BaseClass) {
    ASTClassDecl *cd = dynamic_cast<ASTClassDecl *>(BaseClass);
    if (cd) return cd->lookupMember(name);
  }
  return NULL;
}

ASTClassDecl * ASTClassDecl::superClass() {
  return dynamic_cast<ASTClassDecl *>(BaseClass);
}

bool seen(Base *b, set<Base *> *seenPtrs) {
  bool v = seenPtrs->count(b) > 0;
  seenPtrs->insert(b);
  return v;
}

bool Base::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return false;
}

bool TyArray::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return TyUnresolvedType::hasPointerTo(base, seenPtrs)
      || (Type == base) || (Type && Type->hasPointerTo(base, seenPtrs));
}

bool Types::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  for (auto it = begin(); it != end(); ++it) {
    TyType *t = *it;
    if (t == base || t->hasPointerTo(base, seenPtrs)) return true;
  }
  return false;
}

bool ASTExprs::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  for (auto it = begin(); it != end(); ++it) {
    ASTExpr *e = *it;
    if (e == base || e->hasPointerTo(base, seenPtrs)) return true;
  }
  return false;
}

bool ASTDecls::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  for (auto it = begin(); it != end(); ++it) {
    ASTDecl *d = *it;
    if (d == base || d->hasPointerTo(base, seenPtrs)) return true;
  }
  return false;
}

bool ASTStmts::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  for (auto it = begin(); it != end(); ++it) {
    ASTStmt *s = *it;
    if (s == base || s->hasPointerTo(base, seenPtrs)) return true;
  }
  return false;
}

bool ASTUnaryExpr::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Left == base) || (Left && Left->hasPointerTo(base, seenPtrs));
}

bool ASTBinaryExpr::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Left == base) || (Left && Left->hasPointerTo(base, seenPtrs))
      || (Right == base) || (Right && Right->hasPointerTo(base, seenPtrs));
}

bool ASTFieldExpr::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Left == base) || (Left && Left->hasPointerTo(base, seenPtrs));
}

bool ASTBuiltinPrint::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Expr == base) || (Expr && Expr->hasPointerTo(base, seenPtrs));
}

bool ASTBuiltinNew::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Type == base) || (Type && Type->hasPointerTo(base, seenPtrs));
}

bool ASTBuiltinNewArray::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Expr == base) || (Expr && Expr->hasPointerTo(base, seenPtrs))
      || (Type == base) || (Type && Type->hasPointerTo(base, seenPtrs));
}

bool ASTFunctionCall::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Actuals == base) || (Actuals && Actuals->hasPointerTo(base, seenPtrs));
}

bool ASTMemberFunctionCall::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTExpr::hasPointerTo(base, seenPtrs)
      || (Object == base) || (Object && Object->hasPointerTo(base, seenPtrs))
      || (Actuals == base) || (Actuals && Actuals->hasPointerTo(base, seenPtrs));
}

bool ASTIf::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTStmt::hasPointerTo(base, seenPtrs)
      || (Cond == base) || (Cond && Cond->hasPointerTo(base, seenPtrs))
      || (Then == base) || (Then && Then->hasPointerTo(base, seenPtrs))
      || (ElsePart == base) || (ElsePart && ElsePart->hasPointerTo(base, seenPtrs));
}

bool ASTWhile::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTStmt::hasPointerTo(base, seenPtrs)
      || (Cond == base) || (Cond && Cond->hasPointerTo(base, seenPtrs))
      || (Stmt == base) || (Stmt && Stmt->hasPointerTo(base, seenPtrs));
}

bool ASTFor::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTStmt::hasPointerTo(base, seenPtrs)
      || (Init == base) || (Init && Init->hasPointerTo(base, seenPtrs))
      || (Cond == base) || (Cond && Cond->hasPointerTo(base, seenPtrs))
      || (Incr == base) || (Incr && Incr->hasPointerTo(base, seenPtrs))
      || (Stmt == base) || (Stmt && Stmt->hasPointerTo(base, seenPtrs));
}

bool ASTReturn::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTStmt::hasPointerTo(base, seenPtrs)
      || (Expr == base) || (Expr && Expr->hasPointerTo(base, seenPtrs));
}

bool ASTBlock::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTStmt::hasPointerTo(base, seenPtrs)
      || (Decls == base) || (Decls && Decls->hasPointerTo(base, seenPtrs))
      || (Stmts == base) || (Stmts && Stmts->hasPointerTo(base, seenPtrs));
}

bool ASTVarDecl::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTDecl::hasPointerTo(base, seenPtrs)
      || (Type == base) || (Type && Type->hasPointerTo(base, seenPtrs));
}

bool ASTFunctionDecl::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  if (seen(this, seenPtrs)) return false;
  return ASTDecl::hasPointerTo(base, seenPtrs)
      || (ReturnType == base) || (ReturnType && ReturnType->hasPointerTo(base, seenPtrs))
      || (Formals == base) || (Formals && Formals->hasPointerTo(base, seenPtrs))
      || (Block == base) || (Block && Block->hasPointerTo(base, seenPtrs));
}

bool ASTClassDecl::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  if (seen(thisBase, seenPtrs)) return false;
  return thisBase == base
      || ASTDecl::hasPointerTo(base, seenPtrs)
      || TyNamedType::hasPointerTo(base, seenPtrs)
      || (BaseClass == base) || (BaseClass && BaseClass->hasPointerTo(base, seenPtrs))
      || (Interfaces == base) || (Interfaces && Interfaces->hasPointerTo(base, seenPtrs))
      || (Fields == base) || (Fields && Fields->hasPointerTo(base, seenPtrs));
}

bool TyPrototype::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  if (seen(thisBase, seenPtrs)) return false;
  return thisBase == base
      || ASTDecl::hasPointerTo(base, seenPtrs)
      || TyType::hasPointerTo(base, seenPtrs)
      || (ReturnType == base) || (ReturnType && ReturnType->hasPointerTo(base, seenPtrs))
      || (Formals == base) || (Formals && Formals->hasPointerTo(base, seenPtrs));
}

bool TyInterface::hasPointerTo(Base *base, set<Base *> *seenPtrs) {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  if (seen(thisBase, seenPtrs)) return false;
  return thisBase == base
      || ASTDecl::hasPointerTo(base, seenPtrs)
      || TyNamedType::hasPointerTo(base, seenPtrs)
      || (Prototypes == base) || (Prototypes && Prototypes->hasPointerTo(base, seenPtrs));
}

string cycleImmuneToStr(Base *from, Base *to) {
  if (!from) return "<NULL>";

  set<Base *> *seenSet = new set<Base *>();
  if (from->hasPointerTo(to, seenSet)) {
    return "<CYCLICAL>";
  }
  delete seenSet;
  return from->toString();
}

/* Tabbing */

static int TabSize = 0;
static int TabWidth = 2;

string tab() {
  string str = "";
  for (int i = 0; i < TabWidth * TabSize; i++) str += " ";
  return str;
}

void indent() {
  TabSize++;
}

void unindent() {
  TabSize--;
}

/*
  toString implementations for AST list classes
*/

string ASTExprs::toString() {
  if (size() == 0)
    return "[]";

  stringstream ss;
  ss << "[";
  indent();
  
  for (iterator it = begin(); it != end(); it++) {
    if (it != begin())
      ss << ", ";
    
    assert (*it != NULL);
    ss << (*it)->toString();
  }
  
  unindent();
  ss << "]";
  return ss.str();
}

string ASTDecls::toString() {
  if (size() == 0)
    return "[]";

  stringstream ss;
  ss << "[";
  indent();

  for (iterator it = begin(); it != end(); it++) {
    assert (*it != NULL);
    ss << endl << tab() << (*it)->toString();
  }

  unindent();
  ss << endl << tab() << "]";
  return ss.str();
}

string ASTStmts::toString() {
  if (size() == 0)
    return "[]";

  stringstream ss;
  ss << "[";
  indent();
  
  for (iterator it = begin(); it != end(); it++) {
    assert (*it != NULL);
    ss << endl << tab() << (*it)->toString();
  }
  
  unindent();
  ss << endl << tab() << "]";
  return ss.str();
}

string Types::toString() {
  if (size() == 0)
    return "[]";

  stringstream ss;
  ss << "[";
  indent();
  
  for (iterator it = begin(); it != end(); it++) {
    assert (*it != NULL);
    ss << endl << tab() << (*it)->toString();
  }
  
  unindent();
  ss << endl << tab() << "]";
  return ss.str();
}

/*
  toString implementations for abstract AST types
*/

string ASTDecl::toString() {
  return "A decl";
}

string ASTStmt::toString() {
  return "A stmt";
}

string ASTExpr::toString() {
  return "<empty expression>";
}

string TyType::toString() {
  return "A type";
}

string TyUnknown::toString() {
  return "<Unknown Type>";
}

string TyUnresolvedType::toString() {
  return "<Unresolved Type>";
}

/*
  Output Operator implementations for ASTs.
*/

ostream& operator << (ostream& stream, AST* ast) {
  if (ast == NULL)
    stream << "<NULL>";
  else
    stream << ast->toString();
  return stream;
}

ostream& operator << (ostream& stream, TyType* type) {
  if (type == NULL)
    stream << "<NULL>";
  else
    stream << type->toString();
  return stream;
}

ostream& operator << (ostream& stream, ASTOp op) {
  switch(op) {
  default:
    stream << (char)op;
    break;

  case OpLessEqual:
    stream << "<=";
    break;

  case OpGreaterEqual:
    stream << ">=";
    break;

  case OpEqual:
    stream << "==";
    break;

  case OpNotEqual:
    stream << "!=";
    break;

  case OpLogicalAnd:
    stream << "&&";
    break;

  case OpLogicalOr:
    stream << "||";
    break;

  case OpIndex:
    stream << "[]";
    break;

  }  
  return stream;
}

ostream& operator << (ostream& stream, ASTDecls* decls) {
  if (decls == NULL)
    stream << "<NULL>";
  else
    stream << decls->toString();
  return stream;
}

ostream& operator << (ostream& stream, ASTStmts* stmts) {
  if (stmts == NULL)
    stream << "<NULL>";
  else
    stream << stmts->toString();
  return stream;
}

ostream& operator << (ostream& stream, ASTExprs* exprs) {
  if (exprs == NULL)
    stream << "<NULL>";
  else
    stream << exprs->toString();
  return stream;
}

ostream& operator << (ostream& stream, Types* types) {
  if (types == NULL)
    stream << "<NULL>";
  else
    stream << types->toString();
  return stream;
}

/*
  toString implementations for concrete types
*/

string ASTUnaryExpr::toString() {
  stringstream ss;
  ss << "ASTUnaryExpr(" << Op << ", " << cycleImmuneToStr(Left, this) << ")";
  return ss.str();
}

string ASTBinaryExpr::toString() {
  stringstream ss;
  ss << "ASTBinaryExpr(" << Op << ", ";
  ss << cycleImmuneToStr(Left, this) << ", ";
  ss << cycleImmuneToStr(Right, this) << ")";
  return ss.str();
}

string ASTFieldExpr::toString() {
  stringstream ss;
  ss << "ASTFieldExpr(" << cycleImmuneToStr(Left, this) << ", " << Right << ")";
  return ss.str();
}

string ASTBuiltinReadInteger::toString() {
  return "ASTBuiltinReadInteger";
}

string ASTBuiltinPrint::toString() {
  stringstream ss;
  ss << "ASTBuiltinPrint(" << cycleImmuneToStr(Expr, this) << ")";
  return ss.str();
}

string ASTBuiltinReadLine::toString() {
  return "ASTBuiltinReadLine";
}

string ASTBuiltinNew::toString() {
  stringstream ss;
  ss << "ASTBuiltinNew(" << cycleImmuneToStr(Type, this) << ")";
  return ss.str();
}

string ASTBuiltinNewArray::toString() {
  stringstream ss;
  ss << "ASTBuiltinNewArray(" << cycleImmuneToStr(Expr, this);
  ss << ", " << cycleImmuneToStr(Type, this) << ")";
  return ss.str();
}

string ASTFunctionCall::toString() {
  stringstream ss;
  ss << "ASTFunctionCall(" << Name << ", " << cycleImmuneToStr(Actuals, this) << ")";
  return ss.str();
}

string ASTMemberFunctionCall::toString() {
  stringstream ss;
  ss << "ASTMemberFunctionCall(" << cycleImmuneToStr(Object, this) << ", ";
  ss << MemberName << ", " << cycleImmuneToStr(Actuals, this) << ")";
  return ss.str();
}

string ASTIntConst::toString() {
  stringstream ss;
  ss << Value;
  return ss.str();
}

string ASTDoubleConst::toString() {
  stringstream ss;
  ss << Value;
  return ss.str();
}

string ASTBoolConst::toString() {
  return Value ? "true" : "false";
}

string ASTStringConst::toString() {
  stringstream ss;
  ss << '"' << Value << '"';
  return ss.str();
}

string ASTNullConst::toString() {
  return "ASTNull";
}

string ASTThis::toString() {
  return "ASTThis";
}

string ASTSuper::toString() {
  return "ASTSuper";
}

string ASTVariable::toString() {
  stringstream ss;
  ss << "ASTVariable(" << Name << ")";
  return ss.str();
}

string ASTIf::toString() {
  stringstream ss;
  indent();
  ss << "ASTIf(" << cycleImmuneToStr(Cond, this);
  ss << ", " << endl << tab() << cycleImmuneToStr(Then, this);
  ss << ", " << endl << tab() << cycleImmuneToStr(ElsePart, this) << ")";
  unindent();
  return ss.str();
}

string ASTWhile::toString() {
  stringstream ss;
  indent();
  ss << "ASTWhile(" << cycleImmuneToStr(Cond, this) << ", " << endl << tab() << cycleImmuneToStr(Stmt, this) << ")";
  unindent();
  return ss.str();
}

string ASTFor::toString() {
  stringstream ss;
  indent();
  ss << "ASTFor(" << cycleImmuneToStr(Init, this) << ", " << cycleImmuneToStr(Cond, this) << ", " << cycleImmuneToStr(Incr, this) << ", " << endl << tab() << cycleImmuneToStr(Stmt, this) << ")";
  unindent();
  return ss.str();
}

string ASTBreak::toString() {
  return "ASTBreak";
}

string ASTReturn::toString() {
  stringstream ss;
  ss << "ASTReturn(" << cycleImmuneToStr(Expr, this) << ")";
  return ss.str();
}

string ASTBlock::toString() {
  stringstream ss;
  ss << "ASTBlock(";
  ss << "Decls = " << cycleImmuneToStr(Decls, this) << ", ";
  ss << "Stmts = " << cycleImmuneToStr(Stmts, this);
  ss << ")";
  return ss.str();
}

string TyNull::toString() {
  return "TyNull";
}

string TyInt::toString() {
  return "TyInt";
}

string TyDouble::toString() {
  return "TyDouble";
}

string TyBool::toString() {
  return "TyBool";
}

string TyString::toString() {
  return "TyString";
}

string TyVoid::toString() {
  return "TyVoid";
}

string TyName::toString() {
  stringstream ss;
  ss << "TyName(" << Name << ")";
  return ss.str();
}

string TyArray::toString() {
  stringstream ss;
  ss << "TyArray(" << cycleImmuneToStr(Type, this) << ")";
  return ss.str();
}

string ASTVarDecl::toString() {
  stringstream ss;
  ss << "ASTVarDecl(" << cycleImmuneToStr(Type, this) << ", " << Name << ")";
  return ss.str();
}

string ASTFunctionDecl::toString() {
  stringstream ss;
  ss << "ASTFunctionDecl(" << endl;
  indent();
  ss << tab() << "Type = " << cycleImmuneToStr(ReturnType, this) << ", " << endl;
  ss << tab() << "Name = " << Name << "," << endl;
  ss << tab() << "Formals = " << cycleImmuneToStr(Formals, this) << ", " << endl;
  ss << tab() << "Block = " << cycleImmuneToStr(Block, this) << ")";
  unindent();
  return ss.str();
}

string ASTClassDecl::toString() {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  stringstream ss;
  ss << "ASTClassDecl(" << Name << ", " << cycleImmuneToStr(BaseClass, thisBase) << ", " << cycleImmuneToStr(Interfaces, thisBase) << ", " << cycleImmuneToStr(Fields, thisBase) << ")";
  return ss.str();
}

string TyInterface::toString() {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  stringstream ss;
  ss << "TyInterface(" << Name << ", " << cycleImmuneToStr(Prototypes, thisBase) << ")";
  return ss.str();
}

string TyPrototype::toString() {
  Base *thisBase = dynamic_cast<Base *>(dynamic_cast<TyType *>(this));
  stringstream ss;
  ss << "TyPrototypeInterface(" << Name << ", " << cycleImmuneToStr(ReturnType, thisBase) << ", " << cycleImmuneToStr(Formals, thisBase) << ")";
  return ss.str();
}

