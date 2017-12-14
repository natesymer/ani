%{
#include <iostream>

using namespace std;
  
#include "scanner.h"
#include "AST.h"
#include "Types.h"
#include "Token.h"

typedef Token *decaf_ltype;
#define YYLTYPE decaf_ltype
#define YYDEBUG 1
void yyerror(const char* message);

/* The final AST to return. */
ASTDecls *tree = new ASTDecls();
%}

%code provides {
  ASTDecls *parser(FILE *input);
}

%error-verbose

%precedence '='
%left TokOR
%left TokAND
%left TokEQUAL TokNOTEQUAL
%nonassoc '<' TokLESSEQUAL '>' TokGREATEREQUAL
%left '+' '-'
%left '*' '/' '%'
%precedence '!' EXCLAM // used to be %right
// %left '[' '.' // apparently not needed... huh

%union {
  Token *token;
  TokString *tok_string;
  TokIdent *tok_ident;
  TokInt *tok_int;
  TokDouble *tok_double;
  TokBool *tok_bool;
  AST *ast;
  ASTDecl *ast_decl;
  ASTStmt *ast_stmt;
  ASTExpr *ast_expr;
  ASTExprs *ast_exprs;
  ASTDecls *ast_decls;
  ASTStmts *ast_stmts;
  ASTVariable *ast_variable;
  ASTIf *ast_if;
  ASTWhile *ast_while;
  ASTFor *ast_for;
  ASTBreak *ast_break;
  ASTReturn *ast_return;
  ASTBlock *ast_block;
  ASTVarDecl *ast_var_decl;
  ASTFunctionDecl *ast_fn_decl;
  ASTClassDecl *ast_class_decl;
  Types *types;
  TyType *ty_type;
  TyPrototype *ty_prototype;
  TyInterface *ty_interface;
}

%type <ast_decl> Decl Field
%type <ast_decls> _CDFields _FieldPlus _VariableDeclPlus _VariableCSV
%type <ast_class_decl> ClassDecl
%type <ast_fn_decl> FunctionDecl
%type <ast_var_decl> Variable VariableDecl
%type <ast_exprs> OpsCSV
%type <ast_stmt> Stmt MatchedIf UnmatchedIf
%type <ast_stmts> _StmtPlus
%type <ast_block> StmtBlock
%type <ast_expr> Constant Expr Ops OpsLValue OpsEnd
%type <ty_interface> InterfaceDecl
%type <ty_type> Type SingletonType
%type <ty_prototype> Prototype
%type <types> _PrototypePlus _TokIDCSV

%token <token> TokAND
%token <token> TokBOOL
%token <token> TokBREAK
%token <tok_bool> TokBoolConst
%token <token> TokCLASS
%token <token> TokDOUBLE
%token <tok_double> TokDoubleConst
%token <token> TokELSE
%token <token> TokEQUAL
%token <token> TokEXTENDS
%token <token> TokFOR
%token <token> TokGREATEREQUAL
%token <tok_ident> TokID
%token <token> TokIF
%token <token> TokIMPLEMENTS
%token <token> TokINT
%token <token> TokINTERFACE
%token <tok_int> TokIntConst
%token <token> TokLESSEQUAL
%token <token> TokNEW
%token <token> TokNEWARRAY
%token <token> TokNOTEQUAL
%token <token> TokNULL
%token <token> TokOR
%token <token> TokPRINT
%token <token> TokREADINTEGER
%token <token> TokREADLINE
%token <token> TokRETURN
%token <token> TokSTRING
%token <tok_string> TokStringConst
%token <token> TokTHIS
%token <tok_ident> TokTYPEID
%token <token> TokVOID
%token <token> TokWHILE

%%

/* start state */
Program : Program Decl { tree->push_back($2); }
        | Decl         { tree->push_back($1); }
        ;

Decl : VariableDecl  { $$ = $1; }
     | FunctionDecl  { $$ = $1; }
     | ClassDecl     { $$ = $1; }
     | InterfaceDecl { $$ = $1; }
     ;

ClassDecl : TokCLASS TokID TokEXTENDS TokID TokIMPLEMENTS _TokIDCSV _CDFields { $$ = new ASTClassDecl($2->name(), new TyName($4->name()), $6, $7); } 
          | TokCLASS TokID                  TokIMPLEMENTS _TokIDCSV _CDFields { $$ = new ASTClassDecl($2->name(), NULL, $4, $5); }
          | TokCLASS TokID TokEXTENDS TokID                         _CDFields { $$ = new ASTClassDecl($2->name(), new TyName($4->name()), new Types(), $5); }
          | TokCLASS TokID                                          _CDFields { $$ = new ASTClassDecl($2->name(), NULL, new Types(), $3); }
          ;
_CDFields : '{' '}'            { $$ = new ASTDecls(); }
          | '{' _FieldPlus '}' { $$ = $2; }
          ;
_TokIDCSV : _TokIDCSV ',' TokID { $$ = $1; $$->push_back(new TyName($3->name())); }
          | TokID               { $$ = new Types(); $$->push_back(new TyName($1->name())); }
          ;

InterfaceDecl : TokINTERFACE TokID '{' '}'                { $$ = new TyInterface($2->name(), new Types()); }
              | TokINTERFACE TokID '{' _PrototypePlus '}' { $$ = new TyInterface($2->name(), $4); }
              ;

FunctionDecl : TokVOID TokID '(' ')' StmtBlock              { $$ = new ASTFunctionDecl(new TyVoid(), $2->name(), new ASTDecls(), $5); }
             | TokVOID TokID '(' _VariableCSV ')' StmtBlock { $$ = new ASTFunctionDecl(new TyVoid(), $2->name(), $4, $6); }
             | Type TokID '(' ')' StmtBlock                 { $$ = new ASTFunctionDecl($1, $2->name(), new ASTDecls(), $5); }
             | Type TokID '(' _VariableCSV ')' StmtBlock    { $$ = new ASTFunctionDecl($1, $2->name(), $4, $6); }
             ;

Prototype : TokVOID TokID '(' ')' ';'              { $$ = new TyPrototype($2->name(), new TyVoid(), new ASTDecls()); }
          | TokVOID TokID '(' _VariableCSV ')' ';' { $$ = new TyPrototype($2->name(), new TyVoid(), $4); }
          | Type TokID '(' ')' ';'                 { $$ = new TyPrototype($2->name(), $1, new ASTDecls()); }
          | Type TokID '(' _VariableCSV ')' ';'    { $$ = new TyPrototype($2->name(), $1, $4); }
          ;

_PrototypePlus : _PrototypePlus Prototype { $$ = $1; $$->push_back($2); }
               | Prototype                { $$ = new Types(); $$->push_back($1); }
               ;

Field : VariableDecl { $$ = $1; }
      | FunctionDecl { $$ = $1; }
      ;
_FieldPlus : Field            { $$ = new ASTDecls(); $$->push_back($1); }
           | _FieldPlus Field { $$ = $1; $$->push_back($2); }
           ;

Constant : TokIntConst    { $$ = new ASTIntConst($1->value()); }
         | TokDoubleConst { $$ = new ASTDoubleConst($1->value()); }
         | TokBoolConst   { $$ = new ASTBoolConst($1->value()); }
         | TokStringConst { $$ = new ASTStringConst($1->value()); }
         | TokNULL        { $$ = new ASTNullConst(); }
         ;

Ops : OpsLValue '=' Ops       { $$ = new ASTBinaryExpr(OpAssign, $1, $3); }
    | Ops '+' Ops	      { $$ = new ASTBinaryExpr(OpAdd, $1, $3); }
    | Ops '-' Ops	      { $$ = new ASTBinaryExpr(OpSubtract, $1, $3); }
    | Ops '*' Ops	      { $$ = new ASTBinaryExpr(OpMultiply, $1, $3); }
    | Ops '/' Ops	      { $$ = new ASTBinaryExpr(OpDivide, $1, $3); }
    | Ops '%' Ops	      { $$ = new ASTBinaryExpr(OpModulos, $1, $3); }
    | Ops '<' Ops	      { $$ = new ASTBinaryExpr(OpLess, $1, $3); }
    | Ops '>' Ops	      { $$ = new ASTBinaryExpr(OpGreater, $1, $3); }
    | Ops TokLESSEQUAL Ops    { $$ = new ASTBinaryExpr(OpLessEqual, $1, $3); }
    | Ops TokGREATEREQUAL Ops { $$ = new ASTBinaryExpr(OpGreaterEqual, $1, $3); }
    | Ops TokEQUAL Ops	      { $$ = new ASTBinaryExpr(OpEqual, $1, $3); }
    | Ops TokNOTEQUAL Ops     { $$ = new ASTBinaryExpr(OpNotEqual, $1, $3); }
    | Ops TokAND Ops	      { $$ = new ASTBinaryExpr(OpLogicalAnd, $1, $3); }
    | Ops TokOR Ops           { $$ = new ASTBinaryExpr(OpLogicalOr, $1, $3); }
    | '!' Ops                 { $$ = new ASTUnaryExpr(OpNot, $2); }
    | '-' Ops %prec EXCLAM    { $$ = new ASTUnaryExpr(OpSubtract, $2); }
    | OpsLValue
    ;
OpsLValue : OpsLValue '.' TokID                { $$ = new ASTFieldExpr($1, $3->name()); }
          // FIXME: these next two derivations do not belong under OpsLValue
          // Who gives a fuck if this is slightly wrong? It gives me
          // more time to call Annalisa!
          | OpsLValue '.' TokID '(' ')'        { $$ = new ASTMemberFunctionCall($1, $3->name(), new ASTExprs()); }
          | OpsLValue '.' TokID '(' OpsCSV ')' { $$ = new ASTMemberFunctionCall($1, $3->name(), $5); }
          | OpsLValue '[' Ops ']'              { $$ = new ASTBinaryExpr(OpIndex, $1, $3); }
          | OpsEnd
          ;
OpsEnd : Constant
       | TokTHIS                                   { $$ = new ASTThis(); }
       | TokID                                     { $$ = new ASTVariable($1->name()); }
       | TokID '(' ')'                             { $$ = new ASTFunctionCall($1->name(), new ASTExprs()); }
       | TokID '(' OpsCSV ')'                      { $$ = new ASTFunctionCall($1->name(), $3); }
       | TokREADINTEGER '(' ')'                    { $$ = new ASTBuiltinReadInteger(); }
       | TokREADLINE '(' ')'                       { $$ = new ASTBuiltinReadLine(); }
       | TokNEW '(' SingletonType ')'              { $$ = new ASTBuiltinNew($3); }
       | TokNEWARRAY '(' Ops ',' SingletonType ')' { $$ = new ASTBuiltinNewArray($3, $5); }
       | TokPRINT '(' OpsCSV ')'                   { $$ = new ASTBuiltinPrint($3); }
       | '(' Ops ')'                               { $$ = $2; }
       ;

OpsCSV : Ops            { $$ = new ASTExprs(); $$->push_back($1); }
       | OpsCSV ',' Ops { $$ = new ASTExprs(*$1); $$->push_back($3); }
       ;

Expr : %empty    { $$ = new ASTExpr(); }
     | Ops
     ;

Variable : Type TokID { $$ = new ASTVarDecl($1, $2->name()); }
         ;
_VariableCSV : Variable                  { $$ = new ASTDecls(); $$->push_back($1); }
             | _VariableCSV ',' Variable { $$ = new ASTDecls(*$1); $$->push_back($3); }
             ;

VariableDecl : Variable ';' { $$ = $1; }
             ;
_VariableDeclPlus : _VariableDeclPlus VariableDecl { $$ = new ASTDecls(*$1); $$->push_back($2); }
                  | VariableDecl                   { $$ = new ASTDecls(); $$->push_back($1); }
                  ;

// CITE: http://marvin.cs.uidaho.edu/Teaching/CS445/danglingElse.html
// DESC: Obtained better idea of what a less ambiguous If/Stmt grammar should be.
Stmt : MatchedIf
     | UnmatchedIf
     ;

_StmtPlus : Stmt           { $$ = new ASTStmts(); $$->push_back($1); }
          | _StmtPlus Stmt { $$ = new ASTStmts(*$1); $$->push_back($2); }
          ;

MatchedIf : TokIF '(' Expr ')' MatchedIf TokELSE MatchedIf  { $$ = new ASTIf($3, $5, $7); }
          | TokWHILE '(' Expr ')' MatchedIf                 { $$ = new ASTWhile($3, $5); }
          | Expr ';'                                        { $$ = $1; }
          | TokBREAK ';'                                    { $$ = new ASTBreak(); }
          | TokRETURN Expr ';'                              { $$ = new ASTReturn($2); }
          | StmtBlock                                       { $$ = $1; }
          | TokFOR '(' Expr ';' Expr ';' Expr ')' MatchedIf { $$ = new ASTFor($3, $5, $7, $9); }
          ;

UnmatchedIf : TokIF '(' Expr ')' Stmt                           { $$ = new ASTIf($3, $5, new ASTExpr()); }
            | TokIF '(' Expr ')' MatchedIf TokELSE UnmatchedIf  { $$ = new ASTIf($3, $5, $7); }
            | TokWHILE '(' Expr ')' UnmatchedIf                 { $$ = new ASTWhile($3, $5); }
            | TokFOR '(' Expr ';' Expr ';' Expr ')' UnmatchedIf { $$ = new ASTFor($3, $5, $7, $9); }
            ;

StmtBlock : '{' '}'                             { $$ = new ASTBlock(new ASTDecls(), new ASTStmts());  }
          | '{' _StmtPlus '}'                   { $$ = new ASTBlock(new ASTDecls(), $2); }
          | '{' _VariableDeclPlus _StmtPlus '}' { $$ = new ASTBlock($2, $3); }
          | '{' _VariableDeclPlus '}'           { $$ = new ASTBlock($2, new ASTStmts()); }
          ;

Type : Type '[' ']' { $$ = new TyArray($1); }
     | TokINT       { $$ = new TyInt(); }
     | TokDOUBLE    { $$ = new TyDouble(); }
     | TokBOOL      { $$ = new TyBool(); }
     | TokSTRING    { $$ = new TyString(); }
     | TokTYPEID    { $$ = new TyName($1->name()); }
     ;

/* Same as Type, but uses a non-type ID */
SingletonType : SingletonType '[' ']' { $$ = new TyArray($1); }
              | TokINT                { $$ = new TyInt(); }
              | TokDOUBLE             { $$ = new TyDouble(); }
              | TokBOOL               { $$ = new TyBool(); }
              | TokSTRING             { $$ = new TyString(); }
              | TokID                 { $$ = new TyName($1->name()); }
              ;
%%

ASTDecls * parser(FILE *input) {
  yyin = input;
  return (yyparse() != 0 || tree == NULL) ? NULL : tree;
}

void yyerror(const char* message) {
  if (yylval.token) {
    cout << yylval.token->lineText() << endl;
    for (int i = 1; i < yylval.token->column(); i++) cerr << "-";
    cout << "^" << endl;
    cout << "Line " << yylval.token->line() << ": " << message << endl;
  } else {
    cout << "End-of-file encountered during parse" << endl;
  }
  cout << "Your program has syntax errors" << endl;
  exit(1);
}

