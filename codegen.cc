#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include "AST.h"
#include "cgscope.h"

/*
  TODO:
  - Interface Variables
*/

using namespace std;

int labelNo = 0;
string getLabel() {
  stringstream ss;
  ss << "LABEL_" << labelNo++;
  return ss.str();
}

void codegenStmt(ASTStmt *s, CGScope *gs);
void codegenExpr(ASTExpr *e, CGScope *gs);
string javaForType(TyType *ty);
string javaTypePrefix(TyType *ty);
int stackUseOpAssign(ASTOp op, ASTExpr *to, ASTExpr *from, CGScope *gs);
int exprStackUse(ASTExpr *e, CGScope *gs);

void codegenBuiltinPrint(ASTBuiltinPrint *p, CGScope *gs) {
  for (auto it = p->Expr->begin();
       it != p->Expr->end();
       ++it) {
    ASTExpr *e = *it;
    *gs << "\tgetstatic\tjava/lang/System/out Ljava/io/PrintStream;\n";
    codegenExpr(e, gs);
    *gs << "\tinvokevirtual\tjava/io/PrintStream/print(";
    *gs << javaForType(e->ExprType) << ")V\n";
  }
}

void codegenBoolConst(ASTBoolConst *bc, CGScope *gs) {
  *gs << (bc->Value ? "\ticonst_1\n" : "\ticonst_0\n");
}

void codegenIntConst(ASTIntConst *i, CGScope *gs) {
  if (i->Value == -1)                      *gs << "\ticonst_m1\n";
  else if (i->Value >= 0 && i->Value <= 5) *gs << "\ticonst_" << i->Value << "\n";
  else                                     *gs << "\tldc_w\t" << i->Value << "\n";
}

void codegenDoubleConst(ASTDoubleConst *dc, CGScope *gs) {
  *gs << "\tldc2_w\t" << dc->Value << "\n";
}

int stackUseOp(ASTOp op, ASTExpr *left, ASTExpr *right, CGScope *gs) {
  switch (op) {
    case OpLogicalAnd: return 0;
    case OpLogicalOr: return 0;
    case OpSubtract: return (!right ? 1 : 0);
    case OpAdd:
    case OpMultiply:
    case OpDivide:
    case OpModulos:
      return 0;
    case OpGreater:
    case OpGreaterEqual:
    case OpLess:
    case OpLessEqual:
    case OpNotEqual:
    case OpEqual:
    case OpNot:
      return 0;
    case OpIndex: return 0;
    case OpAssign: return stackUseOpAssign(op, left, right, gs);
    default: return 0;
  }
}

void codegenBooleanOpBody(string elseLabel, CGScope *gs) {
  string endLabel = getLabel();
  *gs << "\ticonst_0\n";
  *gs << "\tgoto\t" << endLabel << "\n";
  *gs << elseLabel << ":\n";
  *gs << "\ticonst_1\n";
  *gs << endLabel << ":\n";
}

void codegenBooleanOp(ASTExpr *left, ASTExpr *right, string op, CGScope *gs) {
  string trueLabel = getLabel();
  string tcode = javaTypePrefix(left->ExprType);
  if (tcode == "d") { // doubles
    *gs << "\t" << tcode << "cmpg\n";
    *gs << "\tif" << op << "\t" << trueLabel << "\n";
  } else if (tcode == "i" || tcode == "a") { // integers, strings, and objects
    *gs << "\tif_" << tcode << "cmp" << op << "\t" << trueLabel << "\n";
  }

  codegenBooleanOpBody(trueLabel, gs);
}

int stackUseOpAssign(ASTOp op, ASTExpr *to, ASTExpr *from, CGScope *gs) {
  ASTBinaryExpr *arryIdx = NULL;
  ASTVariable *var = NULL;
  ASTFieldExpr *field = NULL;
  if ((arryIdx = dynamic_cast<ASTBinaryExpr *>(to))
      && arryIdx->Op == OpIndex) {
    ASTExpr *ary = arryIdx->Left;
    return exprStackUse(ary, gs)
	 + exprStackUse(arryIdx->Right, gs)
         + exprStackUse(from, gs);
  } else if ((var = dynamic_cast<ASTVariable *>(to))) {
    int use = exprStackUse(from, NULL);
    int i = gs->variablePool->index(var->Name);
    if (i == -1) {
      ASTClassableDecl *clsable = dynamic_cast<ASTClassableDecl *>(var->Decl);
      ASTClassDecl *cls = clsable->Class; 
      bool isStatic = !cls || cls->isGlobal;
      if (!isStatic) use += exprStackUse(var, gs);
    }
    return use;
  } else if ((field = dynamic_cast<ASTFieldExpr *>(to))) {
    int use = exprStackUse(field->Left, gs);
    ASTClassDecl *cls = dynamic_cast<ASTClassDecl *>(field->_LeftDecl);
    bool isStatic = !cls || cls->isGlobal;
    if (!isStatic) use += exprStackUse(field->Left, gs);
    return use;
  }
  return 0;
}

void codegenAssign(ASTExpr *to, ASTExpr *from, CGScope *gs) {
  ASTBinaryExpr *arryIdx = NULL;
  ASTVariable *var = NULL;
  ASTFieldExpr *field = NULL;
  if ((arryIdx = dynamic_cast<ASTBinaryExpr *>(to))
      && arryIdx->Op == OpIndex) {
    ASTExpr *ary = arryIdx->Left;
    TyArray *aryType = dynamic_cast<TyArray *>(ary->ExprType);
    if (aryType) {
      codegenExpr(ary, gs);
      codegenExpr(arryIdx->Right, gs);
      codegenExpr(from, gs);
      *gs << "\t" << javaTypePrefix(aryType->Type) << "astore\n";
    }
  } else if ((var = dynamic_cast<ASTVariable *>(to))) {
    int i = gs->variablePool->index(var->Name);
    if (i != -1) {
      codegenExpr(from, gs);
      *gs << "\t" << javaTypePrefix(var->ExprType) << "store" << ((i <= 3) ? "_" : " ") << i << "\n";
    } else {
      ASTClassableDecl *clsable = dynamic_cast<ASTClassableDecl *>(var->Decl);
      
      ASTClassDecl *cls = clsable->Class; 
      bool isStatic = !cls || cls->isGlobal;

      if (!isStatic) codegenExpr(var, gs);
      codegenExpr(from, gs);

      *gs << "\tput";
      if (isStatic) *gs << "static";
      else          *gs << "field";
      *gs << "\t" << cls->Name << "/" << var->Name << " " << javaForType(var->ExprType) << "\n";
    }
  } else if ((field = dynamic_cast<ASTFieldExpr *>(to))) {
    ASTClassDecl *cls = dynamic_cast<ASTClassDecl *>(field->_LeftDecl);
    bool isStatic = !cls || cls->isGlobal;
    if (!isStatic) codegenExpr(field->Left, gs);
    codegenExpr(from, gs);
    
    *gs << "\tput";
    if (isStatic) *gs << "static";
    else          *gs << "field";
    *gs << "\t" << cls->Name << "/" << field->Right << " ";
    *gs << javaForType(field->_RightDecl->Type) << "\n";
  }
}

void codegenOp(ASTExpr *left, ASTExpr *right, ASTOp op, CGScope *gs) {
  if (op == OpAssign) {
    codegenAssign(left, right, gs);
    return;
  }

  if (!right && op == OpSubtract) *gs << "\ticonst_0\n";
  codegenExpr(left, gs);
  if (right) codegenExpr(right, gs);

  switch (op) {
    case OpLogicalAnd: {
      // Takes two "boolean"s (0 and 1 int consts)
      // leaves a "boolean" on the stack
      *gs << "\tiand\n";
      break;
    }
    case OpLogicalOr: {
      // Takes two "boolean"s (0 and 1 int consts)
      // leaves a "boolean" on the stack
      string onetrue = getLabel();
      string twotrue = getLabel();
      string done = getLabel();
      *gs << "\tifne\t" << onetrue << "\n";
      *gs << "\tifne\t" << twotrue << "\n";
      *gs << "\ticonst_0\n";
      *gs << "\tgoto\t" << done << "\n";
      *gs << onetrue << ":\n";
      *gs << "\tpop\n";
      *gs << twotrue << ":\n";
      *gs << "\ticonst_1\n";
      *gs << done << ":\n";
      break;
    }
    case OpAdd: {
      *gs << "\t" << javaTypePrefix(left->ExprType) << "add\n";
      break;
    }
    case OpSubtract: {
      *gs << "\t" << javaTypePrefix(left->ExprType) << "sub\n";
      break;
    }
    case OpMultiply: {
      *gs << "\t" << javaTypePrefix(left->ExprType) << "mul\n";
      break;
    }
    case OpDivide: {
      *gs << "\t" << javaTypePrefix(left->ExprType) << "div\n";
      break;
    }
    case OpModulos: {
      *gs << "\t" << javaTypePrefix(left->ExprType) << "rem\n";
      break;
    }
    case OpLess: {
      codegenBooleanOp(left, right, "lt", gs);
      break;
    }
    case OpLessEqual: {
      codegenBooleanOp(left, right, "le", gs);
      break;
    }
    case OpGreater: {
      codegenBooleanOp(left, right, "gt", gs);
      break;
    }
    case OpGreaterEqual: {
      codegenBooleanOp(left, right, "ge", gs);
      break;
    }
    case OpEqual: {
      codegenBooleanOp(left, right, "eq", gs);
      break;
    }
    case OpNotEqual: {
      codegenBooleanOp(left, right, "ne", gs);
      break;
    }
    case OpNot: {
      string trueLabel = getLabel();
      *gs << "\tifeq\t" << trueLabel << "\n";
      codegenBooleanOpBody(trueLabel, gs);
      break;
    }
    case OpIndex: {
      TyArray *ary = dynamic_cast<TyArray *>(left->ExprType);
      if (ary) {
        *gs << "\t" << javaTypePrefix(ary->Type) << "aload\n";
      }
      break;
    }
    default: break;
  }
}

void codegenFieldExpr(ASTFieldExpr *fe, CGScope *gs) {
  codegenExpr(fe->Left, gs);
  *gs << "\tgetfield\t" << fe->_LeftDecl->Name;
  *gs << "/" << fe->Right << " " << javaForType(fe->_RightDecl->Type) << "\n";
}

void codegenBuiltinReadInteger(CGScope *gs) {
  *gs << "\tnew java/util/Scanner\n";
  *gs << "\tdup\n";
  *gs << "\tgetstatic\tjava/lang/System/in Ljava/io/InputStream;\n";
  *gs << "\tinvokespecial\tjava/util/Scanner/<init>(Ljava/io/InputStream;)V\n";
  *gs << "\tinvokevirtual\tjava/util/Scanner/nextInt()I\n";
}

void codegenBuiltinReadLine(CGScope *gs) {
  *gs << "\tnew java/util/Scanner\n";
  *gs << "\tdup\n";
  *gs << "\tgetstatic\tjava/lang/System/in Ljava/io/InputStream;\n";
  *gs << "\tinvokespecial\tjava/util/Scanner/<init>(Ljava/io/InputStream;)V\n";
  *gs << "\tinvokevirtual\tjava/util/Scanner/nextLine()Ljava/lang/String;\n";
}

void codegenBuiltinNew(ASTBuiltinNew *n, CGScope *gs) {
  ASTDecl *d = dynamic_cast<ASTDecl *>(n->Type);
  *gs << "\tnew\t" << d->Name << "\n";
  *gs << "\tdup\n";
  *gs << "\tinvokespecial\t" << d->Name << "/<init>()V\n";
}

void codegenBuiltinNewArray(ASTBuiltinNewArray *na, CGScope *gs) {
  codegenExpr(na->Expr, gs);

  int depth = 1;
  for (TyArray *t = dynamic_cast<TyArray *>(na->Type);
       t != NULL;
       t = dynamic_cast<TyArray *>(t->Type), depth++)
    codegenExpr(na->Expr, gs); // TODO: this isn't quite right

  *gs << "\tmultianewarray\t[" << javaForType(na->Type) << "\t" << depth << "\n";
}

void codegenFunctionSignature(string clsname, string fname, ASTDecls *formals, TyType *returnType, CGScope *gs) {
  if (clsname.length() > 0) *gs << clsname << "/";
  *gs << fname << "(";
  for (auto it = formals->begin();
       it != formals->end();
       ++it) {
      ASTVarDecl *vd = dynamic_cast<ASTVarDecl *>(*it);
      *gs << javaForType(vd->Type);
  }
  *gs << ")" << javaForType(returnType);
}

void codegenMemberFunctionCall(ASTMemberFunctionCall *fc, CGScope *gs) {
  codegenExpr(fc->Object, gs);
  if (fc->MemberName == "length") {
    TyType *objType = fc->Object->ExprType;
    if (dynamic_cast<TyString *>(objType))
      *gs << "\tinvokevirtual\tjava/lang/String/length()I\n";
    else if (dynamic_cast<TyArray *>(objType))
      *gs << "\tarraylength\n";
  } else {
    ASTExprs *actuals = fc->Actuals;

    for (auto it = actuals->begin();
         it != actuals->end();
         ++it) codegenExpr(*it, gs);

    ASTDecl *onDecl = dynamic_cast<ASTDecl *>(fc->Object->ExprType);

    ASTFunctionDecl *fn = NULL;
    TyPrototype *ptype = NULL;
    if ((fn = dynamic_cast<ASTFunctionDecl *>(fc->Decl))) {
      *gs << "\tinvokevirtual\t";
      codegenFunctionSignature(fn->Class->Name, fn->Name, fn->Formals, fn->ReturnType, gs);
    } else if ((ptype = dynamic_cast<TyPrototype *>(fc->Decl))) {
      *gs << "\tinvokeinterface\t";
      codegenFunctionSignature(onDecl->Name, ptype->Name, ptype->Formals, ptype->ReturnType, gs);
    }

    if (dynamic_cast<TyInterface *>(onDecl))
      *gs << "\t" << (1 + actuals->size());

    *gs << "\n";
  }
}

void codegenFunctionCall(ASTFunctionCall *fcall, CGScope *gs) {
  ASTDecl *member = NULL;
  if (gs->currentClass
   && !gs->currentClass->isGlobal
   && (member = gs->currentClass->lookupMember(fcall->Name))
   && dynamic_cast<ASTFunctionDecl *>(member)) {
    ASTMemberFunctionCall *mfc = new ASTMemberFunctionCall(new ASTThis(), fcall->Name, fcall->Actuals);
    mfc->Decl = fcall->Decl;
    mfc->ExprType = fcall->ExprType;
    codegenMemberFunctionCall(mfc, gs);
    delete mfc;
  } else {
    ASTFunctionDecl *fn = dynamic_cast<ASTFunctionDecl *>(fcall->Decl);
    ASTExprs *actuals = fcall->Actuals;
   
    for (auto it = actuals->begin();
         it != actuals->end();
         ++it) codegenExpr(*it, gs);
   
    *gs << "\tinvokestatic\t";
    codegenFunctionSignature(fn->Class->Name, fn->Name, fn->Formals, fn->ReturnType, gs);
    *gs << "\n";
  }
}

void codegenVariable(ASTVariable *var, CGScope *gs) {
  ASTDecl *member = NULL;
  int i = gs->variablePool->index(var->Name);
  if (i != -1)
    *gs << "\t" << javaTypePrefix(var->ExprType) << "load" << ((i <= 3) ? "_" : " ") << i << "\n";
  else if (gs->currentClass
   && !gs->currentClass->isGlobal
   && (member = gs->currentClass->lookupMember(var->Name))
   && dynamic_cast<ASTVarDecl *>(member)) {
    ASTFieldExpr *fe = new ASTFieldExpr(new ASTThis(), var->Name);
    fe->_RightDecl = dynamic_cast<ASTVarDecl *>(var->Decl);
    fe->_LeftDecl = gs->currentClass;
    fe->ExprType = var->ExprType;
    codegenFieldExpr(fe, gs);
    delete fe;
  } else {
    ASTClassableDecl *clsable = dynamic_cast<ASTClassableDecl *>(var->Decl);
    ASTClassDecl *cls = clsable->Class;
    bool isStatic = !cls || cls->isGlobal;
    *gs << "\tget";
    if (isStatic) *gs << "static";
    else          *gs << "field";
    *gs << "\t" << cls->Name << "/" << var->Name << " " << javaForType(var->ExprType) << "\n";
  }
}

void codegenExpr(ASTExpr *e, CGScope *gs) {
  ASTMemberFunctionCall *mf = NULL;
  ASTBuiltinPrint *p = NULL;
  ASTStringConst *strc = NULL;
  ASTIntConst *intc = NULL;
  ASTBoolConst *bc = NULL;
  ASTDoubleConst *dc = NULL;
  ASTUnaryExpr *ue = NULL;
  ASTBinaryExpr *be = NULL;
  ASTFieldExpr *fe = NULL;
  ASTBuiltinNew *bn = NULL;
  ASTBuiltinNewArray *bna = NULL;
  ASTFunctionCall *fc = NULL;
  ASTVariable *var = NULL;
  if ((p = dynamic_cast<ASTBuiltinPrint *>(e))) codegenBuiltinPrint(p, gs);
  else if (dynamic_cast<ASTThis *>(e)) *gs << "\taload_0\n";
  else if (dynamic_cast<ASTNullConst *>(e)) *gs << "\taconst_null\n";
  else if ((bc = dynamic_cast<ASTBoolConst *>(e))) codegenBoolConst(bc, gs);
  else if ((strc = dynamic_cast<ASTStringConst *>(e))) *gs << "\tldc\t" << strc->Value << "\n";
  else if ((intc = dynamic_cast<ASTIntConst *>(e))) codegenIntConst(intc, gs);
  else if ((dc = dynamic_cast<ASTDoubleConst *>(e))) codegenDoubleConst(dc, gs);
  else if ((mf = dynamic_cast<ASTMemberFunctionCall *>(e))) codegenMemberFunctionCall(mf, gs);
  else if ((ue = dynamic_cast<ASTUnaryExpr *>(e))) codegenOp(ue->Left, NULL, ue->Op, gs);
  else if ((be = dynamic_cast<ASTBinaryExpr *>(e))) codegenOp(be->Left, be->Right, be->Op, gs);
  else if ((fe = dynamic_cast<ASTFieldExpr *>(e))) codegenFieldExpr(fe, gs);
  else if (dynamic_cast<ASTBuiltinReadInteger *>(e)) codegenBuiltinReadInteger(gs);
  else if (dynamic_cast<ASTBuiltinReadLine *>(e)) codegenBuiltinReadLine(gs);
  else if ((bn = dynamic_cast<ASTBuiltinNew *>(e))) codegenBuiltinNew(bn, gs);
  else if ((bna = dynamic_cast<ASTBuiltinNewArray *>(e))) codegenBuiltinNewArray(bna, gs);
  else if ((fc = dynamic_cast<ASTFunctionCall *>(e))) codegenFunctionCall(fc, gs);
  else if ((var = dynamic_cast<ASTVariable *>(e))) codegenVariable(var, gs);
}

int exprStackUse(ASTExpr *e, CGScope *gs) {
  ASTBuiltinPrint *bp = NULL;
  ASTUnaryExpr *ue = NULL;
  ASTBinaryExpr *be = NULL;
  ASTFieldExpr *fe = NULL;
  ASTFunctionCall *fc = NULL;
  ASTMemberFunctionCall *mfc = NULL;
  ASTVariable *v = NULL;
  ASTBuiltinNewArray *na = NULL;
  if ((bp = dynamic_cast<ASTBuiltinPrint *>(e))) {
    int total = 0;
    for (auto it = bp->Expr->begin();
	 it != bp->Expr->end();
	 ++it) total += exprStackUse(*it, gs) + 1;
    return total;
  }
  else if (dynamic_cast<ASTThis *>(e)) return 1;
  else if (dynamic_cast<ASTNullConst *>(e)) return 1;
  else if (dynamic_cast<ASTStringConst *>(e)) return 1;
  else if (dynamic_cast<ASTIntConst *>(e)) return 1;
  else if (dynamic_cast<ASTDoubleConst *>(e)) return 1;
  else if ((ue = dynamic_cast<ASTUnaryExpr *>(e)))
    return exprStackUse(ue->Left, gs)
         + stackUseOp(ue->Op, ue->Left, NULL, gs);
  else if ((be = dynamic_cast<ASTBinaryExpr *>(e)))
    return exprStackUse(be->Left, gs)
	 + exprStackUse(be->Right, gs)
	 + stackUseOp(be->Op, be->Left, be->Right, gs);
  else if ((fe = dynamic_cast<ASTFieldExpr *>(e))) return exprStackUse(fe->Left, gs);
  else if (dynamic_cast<ASTBuiltinReadInteger *>(e)) return 3;
  else if (dynamic_cast<ASTBuiltinReadLine *>(e)) return 3;
  else if (dynamic_cast<ASTBuiltinNew *>(e)) return 2;
  else if ((na = dynamic_cast<ASTBuiltinNewArray *>(e))) return 1; // TODO fixme
  else if ((fc = dynamic_cast<ASTFunctionCall *>(e))) {
    ASTExprs *es = fc->Actuals;
    int esUse = 0;
    for (auto it = es->begin();
	 it != es->end();
	 ++it) esUse = exprStackUse(*it, gs);
    return esUse + 1;
  } else if ((mfc = dynamic_cast<ASTMemberFunctionCall *>(e))) {
    ASTExprs *es = mfc->Actuals;
    int esUse = 0;
    for (auto it = es->begin();
	 it != es->end();
	 ++it) esUse = exprStackUse(*it, gs);
    return esUse + exprStackUse(mfc->Object, gs);
  } else if ((v = dynamic_cast<ASTVariable *>(e))) return 1;
  else return 0;
}

int stmtStackUse(ASTStmt *s, CGScope *gs) {
  ASTReturn *r = NULL;
  ASTExpr *e = NULL;
  ASTIf *ff = NULL;
  ASTWhile *w = NULL;
  ASTFor *fr = NULL;
  ASTBreak *bk = NULL;
  ASTBlock *bl = NULL;
  if ((r = dynamic_cast<ASTReturn *>(s))) return r->Expr ? exprStackUse(r->Expr, gs) : 0;
  else if ((e = dynamic_cast<ASTExpr *>(s))) return exprStackUse(e, gs);
  else if ((ff = dynamic_cast<ASTIf *>(s))) {
    int a = stmtStackUse(ff->Then, gs);
    int b = ff->ElsePart ? stmtStackUse(ff->ElsePart, gs) : 0;
    return (a > b ? a : b) + exprStackUse(ff->Cond, gs) + 1; // the one is from pushing a bool to compare against
  } else if ((w = dynamic_cast<ASTWhile *>(s))) {
    return exprStackUse(w->Cond, gs) + 1 + stmtStackUse(w->Stmt, gs);
  } else if ((fr = dynamic_cast<ASTFor *>(s))) {
    return exprStackUse(fr->Init, gs)
         + exprStackUse(fr->Incr, gs)
         + exprStackUse(fr->Cond, gs)
         + 1 + stmtStackUse(fr->Stmt, gs);
  } else if ((bk = dynamic_cast<ASTBreak *>(s))) return 0;
  else if ((bl = dynamic_cast<ASTBlock *>(s))) {
    int total = 0;
    for (auto it = bl->Stmts->begin();
	 it != bl->Stmts->end();
	 ++it) total += stmtStackUse(*it, gs);
    return total;
  }
  return 0;
}

void codegenBlock(ASTBlock *b, CGScope *gs) {
  CGScope *subgs = new CGScope(gs);
  subgs->currentBlock = b;
  for (auto it = b->Decls->begin();
       it != b->Decls->end();
       ++it) subgs->variablePool->add(dynamic_cast<ASTVarDecl *>(*it));
  for (auto it = b->Stmts->begin();
       it != b->Stmts->end();
       ++it) codegenStmt(*it, subgs);
  delete subgs;
}

void codegenIf(ASTIf *s, CGScope *gs) {
  string beforeElse = getLabel();
  string afterElse = getLabel();

  codegenExpr(s->Cond, gs); // cond leaves a bool on the stack
  *gs << "\tifeq\t" << beforeElse << "\n";
  codegenStmt(s->Then, gs);
  *gs << beforeElse << ":\n";
  *gs << "\tgoto\t" << afterElse << "\n";
  codegenStmt(s->ElsePart, gs);
  *gs << afterElse << ":\n";
}

void codegenWhile(ASTWhile *w, CGScope *gs) {
  string afterWhile = getLabel();
  string beforeWhile = getLabel();

  gs->breakLabels->push(afterWhile);

  *gs << beforeWhile << ":\n";
  codegenExpr(w->Cond, gs); // cond leaves a bool on the stack
  *gs << "\tifeq\t" << afterWhile << "\n";
  codegenStmt(w->Stmt, gs);
  *gs << "\tgoto\t" << beforeWhile << "\n";
  *gs << afterWhile << ":\n";

  gs->breakLabels->pop();
}

void codegenFor(ASTFor *f, CGScope *gs) {
  string afterFor = getLabel();
  string beforeFor = getLabel();

  gs->breakLabels->push(afterFor);

  codegenExpr(f->Init, gs);
  *gs << beforeFor << ":\n";
  codegenExpr(f->Cond, gs); // cond leaves a bool on the stack
  *gs << "\tifeq\t" << afterFor << "\n";
  codegenStmt(f->Stmt, gs);
  codegenExpr(f->Incr, gs);
  *gs << "\tgoto\t" << beforeFor << "\n";
  *gs << afterFor << ":\n";

  gs->breakLabels->pop();
}

void codegenReturn(ASTReturn *r, CGScope *gs) {
  if (r->Expr) codegenExpr(r->Expr, gs);
  *gs << "\t" << javaTypePrefix(gs->currentFunction->ReturnType) << "return\n";
}

void codegenBreak(ASTBreak *b, CGScope *gs) {
  if (!gs->breakLabels->empty())
    *gs << "\tgoto\t" << gs->breakLabels->top() << "\n";
}

void codegenStmt(ASTStmt *s, CGScope *gs) {
  ASTReturn *r = NULL;
  ASTExpr *e = NULL;
  ASTIf *ff = NULL;
  ASTWhile *w = NULL;
  ASTFor *fr = NULL;
  ASTBreak *br = NULL;
  ASTBlock *bl = NULL;
  if ((r = dynamic_cast<ASTReturn *>(s))) codegenReturn(r, gs);
  else if ((e = dynamic_cast<ASTExpr *>(s))) codegenExpr(e, gs);
  else if ((ff = dynamic_cast<ASTIf *>(s))) codegenIf(ff, gs);
  else if ((w = dynamic_cast<ASTWhile *>(s))) codegenWhile(w, gs);
  else if ((fr = dynamic_cast<ASTFor *>(s))) codegenFor(fr, gs);
  else if ((br = dynamic_cast<ASTBreak *>(s))) codegenBreak(br, gs);
  else if ((bl = dynamic_cast<ASTBlock *>(s))) codegenBlock(bl, gs);
}

int countVarDecls(ASTStmt *s) {
  ASTBlock *blk = NULL;
  ASTIf *ifs = NULL;
  ASTWhile *whl = NULL;
  ASTFor *fr = NULL;
  if ((blk = dynamic_cast<ASTBlock *>(s))) {
    int count = blk->Decls->size(); // assumed to be all var decls
    for (auto it = blk->Stmts->begin();
	 it != blk->Stmts->end();
	 ++it) {
      count += countVarDecls(*it);
    }
    return count;
  } else if ((ifs = dynamic_cast<ASTIf *>(s))) return countVarDecls(ifs->Then)
						    + countVarDecls(ifs->ElsePart);
  else if ((whl = dynamic_cast<ASTWhile *>(s))) return countVarDecls(whl->Stmt);
  else if ((fr = dynamic_cast<ASTFor *>(s))) return countVarDecls(fr->Stmt);
  return 0;
}

void codegenFunctionDecl(ASTFunctionDecl *f, CGScope *gs) {
  CGScope *subgs = new CGScope(gs);
  subgs->currentFunction = f;
  if (f->Class && !f->Class->isGlobal) subgs->variablePool->reserveIndex();

  *gs << ".method\tpublic";
  if (f->Class && !f->Class->isGlobal) *gs << "\t";
  else                                 *gs << " static" << "\t";

  codegenFunctionSignature("", f->Name, f->Formals, f->ReturnType, subgs);
  *gs << "\n";

  for (auto it = f->Formals->begin();
       it != f->Formals->end();
       ++it) subgs->variablePool->add(dynamic_cast<ASTVarDecl *>(*it));

  *gs << "\t.limit stack\t" << stmtStackUse(f->Block, gs) << "\n";
  *gs << "\t.limit locals\t" << (f->Class ? 1 : 0) + f->Formals->size() + countVarDecls(f->Block) << "\n";
  codegenBlock(f->Block, subgs);
  *gs << "\treturn\n.end method\n";

  delete subgs;
}

string javaForType(TyType *ty) {
  TyArray *a = dynamic_cast<TyArray *>(ty);
  TyInterface *i = dynamic_cast<TyInterface *>(ty);
  ASTClassDecl *c = dynamic_cast<ASTClassDecl *>(ty);
  if (a) return "[" + javaForType(a->Type);
  else if (dynamic_cast<TyVoid *>(ty)) return "V";
  else if (dynamic_cast<TyString *>(ty)) return "Ljava/lang/String;";
  else if (dynamic_cast<TyInt *>(ty)) return "I";
  else if (dynamic_cast<TyDouble *>(ty)) return "D";
  else if (dynamic_cast<TyBool *>(ty)) return "Z";
  else if (i) return "L" + i->Name + ";";
  else if (c) return "L" + c->Name + ";";

  // TyName, TyUnknown
  cerr << "No java equivalent for type: " << ty << endl;
  exit(1);
  return "";
}

string javaTypePrefix(TyType *ty) {
  if (dynamic_cast<TyVoid *>(ty))           return "";
  else if (dynamic_cast<TyArray *>(ty))     return "a";
  else if (dynamic_cast<TyString *>(ty))    return "a";
  else if (dynamic_cast<TyInt *>(ty))       return "i";
  else if (dynamic_cast<TyDouble *>(ty))    return "d";
  else if (dynamic_cast<TyBool *>(ty))      return "i";
  else if (dynamic_cast<TyNamedType *>(ty)) return "a";

  // TyName, TyUnknown
  cerr << "No java for type: " << ty << endl;
  exit(1);
  return "";
}

void codegenClassDecl(ASTClassDecl *cd, CGScope *gs) {
  ASTClassDecl *super = dynamic_cast<ASTClassDecl *>(cd->BaseClass);
  *gs << ".class\tpublic " << cd->Name << "\n";
  *gs << ".super\t" << (super ? super->Name : "java/lang/Object") << "\n";

  for (auto it = cd->Interfaces->begin();
       it != cd->Interfaces->end();
       ++it) {
    *gs << ".implements\t" << dynamic_cast<TyInterface *>(*it)->Name << "\n";
  }

  list<ASTFunctionDecl *> fds;
  for (auto it = cd->Fields->begin();
       it != cd->Fields->end();
       ++it) {
    ASTFunctionDecl *fd = NULL;
    ASTVarDecl *vd = NULL;
    if ((fd = dynamic_cast<ASTFunctionDecl *>(*it))) fds.push_back(fd);
    else if ((vd = dynamic_cast<ASTVarDecl *>(*it))) {
      *gs << ".field\t";
      if (!vd->Class || vd->Class->isGlobal) *gs << "static ";
      *gs << vd->Name << "\t" << javaForType(vd->Type) << "\n";
    }
  }

  string superclassName = cd->BaseClass ? dynamic_cast<ASTDecl *>(cd->BaseClass)->Name : "java/lang/Object";

  *gs << ".method\tpublic <init>()V\n";
  *gs << "\t.limit stack\t1\n";
  *gs << "\t.limit locals\t1\n";
  *gs << "\taload_0\n";
  *gs << "\tinvokespecial\t" << superclassName << "/<init>()V\n";
  *gs << "\treturn\n";
  *gs << ".end method\n";

  for (auto it = fds.begin();
       it != fds.end();
       ++it) codegenFunctionDecl(*it, gs);
}

void codegenInterfaceDecl(TyInterface *iface, CGScope *gs) {
  *gs << ".interface\tabstract " << iface->Name << "\n";
  *gs << ".super\tjava/lang/Object\n";
  
  for (auto it = iface->Prototypes->begin();
       it != iface->Prototypes->end();
       ++it) {
    TyPrototype *p  = dynamic_cast<TyPrototype *>(*it);
    *gs << ".method\tpublic abstract " << p->Name << "(";
    
    for (auto ittwo = p->Formals->begin();
         ittwo != p->Formals->end();
         ++ittwo) {
      ASTVarDecl *vd = dynamic_cast<ASTVarDecl *>(*ittwo);
      *gs << javaForType(vd->Type);
    }
    
    *gs << ")" << javaForType(p->ReturnType) << "\n";
    *gs << ".end method\n";
  }
}

list<string> codegen(string dir, string fileName, ASTDecls* tree) {
  ASTFunctionDecl *mainDecl = NULL;

  for (auto it = tree->begin();
       it != tree->end();
       ++it) {
    ASTFunctionDecl *fd = dynamic_cast<ASTFunctionDecl *>(*it);
    if (fd && fd->Name == "main") {
      mainDecl = fd;
      break;
    }
  }

  // Ensure that mainDecl can serve as a JVM entry point.
  if (!mainDecl) {
    cerr << "No top-level main() found." << endl;
    exit(1);
  } else if (mainDecl->Formals->size() == 0) {
    mainDecl->Formals->push_back(new ASTVarDecl(new TyArray(new TyString()), "args"));
  } else if (mainDecl->Formals->size() == 1) {
    ASTVarDecl *vd = dynamic_cast<ASTVarDecl *>(mainDecl->Formals->front());
    TyArray *atype  = dynamic_cast<TyArray *>(vd->Type);
    TyString *strType = dynamic_cast<TyString *>(atype->Type);
    if (!strType) {
      cerr << "invalid main(): argument signature: " << mainDecl->Formals << endl << "\noptions are `void main()' and `void main(string[] args)'." << endl;
      exit(1);
    }
  } else if (!dynamic_cast<TyVoid *>(mainDecl->ReturnType)) {
    cerr << "invalid main(): does not return void." << endl;
    exit(1);
  } else {
    cerr << "main() has too many arguments: " << mainDecl->Formals << endl;
    exit(1);
  }

  list<ASTClassDecl *> classes;
  list<TyInterface *> interfaces;

  ASTDecls *decls = new ASTDecls();

  for (auto it = tree->begin();
       it != tree->end();
       ++it) {
    ASTDecl *d = *it;
    ASTClassDecl *cd = NULL;
    TyInterface *iface = NULL;
    if ((cd = dynamic_cast<ASTClassDecl *>(d))) classes.push_back(cd);
    else if ((iface = dynamic_cast<TyInterface *>(d))) interfaces.push_back(iface);
    else if (dynamic_cast<ASTFunctionDecl *>(d)) decls->push_back(d);
    else if (dynamic_cast<ASTVarDecl *>(d)) decls->push_back(d);
    else {
      cerr << "Invalid declaration: " << d << endl;
      exit(1);
    }
  }

  ASTClassDecl *cd = new ASTClassDecl(fileName, NULL, new Types(), decls);
  cd->isGlobal = true;
  for (auto it = cd->Fields->begin();
       it != cd->Fields->end();
       ++it) {
    ASTClassableDecl *cbd = NULL;
    if ((cbd = dynamic_cast<ASTClassableDecl *>(*it))) cbd->Class = cd;
  }

  classes.push_front(cd);

  list<string> fileNames;

  for (auto it = classes.begin();
       it != classes.end();
       ++it) {
    string n = dir + (*it)->Name + ".jsm";
    FILE *f = fopen(n.c_str(), "w");
    CGScope *s = new CGScope(f);
    s->currentClass = *it;
    codegenClassDecl(*it, s);
    fclose(f);
    delete s;
    fileNames.push_back(n);
  }

  for (auto it = interfaces.begin();
       it != interfaces.end();
       ++it) {
    string n = dir + (*it)->Name + ".jsm";
    FILE *f = fopen(n.c_str(), "w");
    CGScope *s = new CGScope(f);
    s->currentInterface = *it;
    codegenInterfaceDecl(*it, s);
    fclose(f);
    delete s;
    fileNames.push_back(n);
  }

  return fileNames;
}

