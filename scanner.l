%{

#include <iostream>
#include <stdarg.h>
#include "AST.h"
#include "TokenNumbers.h"

void advance_cursor();
int makeToken(int tokid);
int identTokType();
void print_error(const char *errfmt, ...);

#define YYLTYPE Token*

%}

%option noyywrap
%option yylineno
%option nodefault

%x MULTICOMMENT
%x IDSUB
%x IDSUB2

/*

TODO: IDs followed by reserved words are not TYPEIDs

*/

NOTIDENT [^A-Za-z \t\n\r\f]
IDENT [A-Za-z][A-Za-z0-9_]*
WHITESPACE [ \t\n\r\f]
RESERVED (true|false|void|string|extends|this|while|break|ReadInteger|int|class|if|New|ReadLine|double|interface|implements|else|NewArray|bool|null|for|return|Print)
BRACKETS (\[\])*

%%
"=="                             { return makeToken(TokEQUAL); }
"!="                             { return makeToken(TokNOTEQUAL); }
"&&"                             { return makeToken(TokAND); }
"||"                             { return makeToken(TokOR); }
"<="                             { return makeToken(TokLESSEQUAL); }
">="                             { return makeToken(TokGREATEREQUAL); }

{IDENT}                          { yyless(0); BEGIN(IDSUB); }
<IDSUB>{IDENT}/{WHITESPACE}*{BRACKETS}({WHITESPACE}+{RESERVED})? {
                                   int tid = identTokType();
                                   BEGIN(INITIAL);
                                   return makeToken(tid == -1 ? TokID : tid);
                                 }
<IDSUB>{IDENT}/{WHITESPACE}*{BRACKETS}{WHITESPACE}+{IDENT} {
                                   int tid = identTokType();
                                   if (tid == -1) return makeToken(TokTYPEID);
                                   else {
                                     BEGIN(INITIAL);
                                     return makeToken(tid);
                                   }
                                 }
<IDSUB>{WHITESPACE}              { advance_cursor(); }
<IDSUB>.                         { yyless(0); BEGIN(INITIAL); }

[-*/%<>+!\;,.[\](){}=]           { return makeToken(*yytext); }
[0-9]+                           { return makeToken(TokIntConst); }
0[xX][A-Fa-f0-9]+                { return makeToken(TokIntConst); }
[0-9]+\.[0-9]*([Ee][-+]?[0-9]+)? { return makeToken(TokDoubleConst); }

"/*"                             { advance_cursor(); BEGIN(MULTICOMMENT); }
<MULTICOMMENT>\n                 { advance_cursor(); }
<MULTICOMMENT>.                  { advance_cursor(); }
<MULTICOMMENT>"*/"               { advance_cursor(); BEGIN(INITIAL); }

\"[^\"\n]*\"                     { return makeToken(TokStringConst); }
\/\/.*                           { advance_cursor(); }
{WHITESPACE}                     { advance_cursor(); }
.                                { print_error("Illegal character, skipping: '%s'\n", yytext); }
%%

using namespace std;

string *line = new string();
int scanner_line = 1;

// Using the global line variable, calculates the tab-respectful length.
int lineLength() {
  int count = 0; // start at one
  string::iterator it;

  for (it = line->begin(); it < line->end(); ++it) {
    count++;
    if (*it == '\t') count += 8 - (count % 8);
  }
  return count + 1;
}

const char *error_format_prefix = "%d:%d error: ";
void print_error(const char *errfmt, ...) {
  fprintf(stderr, error_format_prefix, scanner_line, lineLength());
  
  va_list argptr;
  va_start(argptr, errfmt);
  vfprintf(stderr, errfmt, argptr); 
  va_end(argptr);
  advance_cursor();
}

// If yytext contains a reserved word, return the matching
// token id. Otherwise, return -1 (0 is EOF).
int identTokType() {
  char *s = yytext;
  if (strcmp(s, "true") == 0) return TokBoolConst;
  if (strcmp(s, "false") == 0) return TokBoolConst;
  if (strcmp(s, "void") == 0) return TokVOID;
  if (strcmp(s, "string") == 0) return TokSTRING;
  if (strcmp(s, "extends") == 0) return TokEXTENDS;
  if (strcmp(s, "this") == 0) return TokTHIS;
  if (strcmp(s, "while") == 0) return TokWHILE;
  if (strcmp(s, "break") == 0) return TokBREAK;
  if (strcmp(s, "ReadInteger") == 0) return TokREADINTEGER;
  if (strcmp(s, "int") == 0) return TokINT;
  if (strcmp(s, "class") == 0) return TokCLASS;
  if (strcmp(s, "if") == 0) return TokIF;
  if (strcmp(s, "New") == 0) return TokNEW;
  if (strcmp(s, "ReadLine") == 0) return TokREADLINE;
  if (strcmp(s, "double") == 0) return TokDOUBLE;
  if (strcmp(s, "interface") == 0) return TokINTERFACE;
  if (strcmp(s, "implements") == 0) return TokIMPLEMENTS;
  if (strcmp(s, "else") == 0) return TokELSE;
  if (strcmp(s, "NewArray") == 0) return TokNEWARRAY;
  if (strcmp(s, "bool") == 0) return TokBOOL;
  if (strcmp(s, "null") == 0) return TokNULL;
  if (strcmp(s, "for") == 0) return TokFOR;
  if (strcmp(s, "return") == 0) return TokRETURN;
  if (strcmp(s, "Print") == 0) return TokPRINT;
  return -1;
}

// NOT REENTRANT!
void advance_cursor() {
  char *lineptr = yytext;
  char *ptr = lineptr;
  char c;
  while ((c = *ptr)) {
    ptr++;
    if (c == '\n') {
      scanner_line++;
      lineptr = ptr;
      line = new string();
    }
  }
  line->append(lineptr);
}

// This won't crash on strings with len < 2 because of how
// C and C++ logical short circuit.
bool isHex(char *s) {
  return s[0] == '0' && (s[1] == 'x' || s[1] == 'X');
}

Token * createTokenRespectingSubclass(int tokid) {
  int scanner_col = lineLength();
  switch (tokid) {
    case TokBoolConst:
      return new TokBool(strcmp(yytext, "true") == 0, line, scanner_line, scanner_col);
    case TokDoubleConst:
      return new TokDouble(string(yytext), line, scanner_line, scanner_col);
    case TokIntConst:
      return new TokInt(string(yytext), line, scanner_line, scanner_col, isHex(yytext));
    case TokID:
    case TokTYPEID:
      return new TokIdent(string(yytext), line, scanner_line, scanner_col);
    case TokStringConst:
      return new TokString(string(yytext), line, scanner_line, scanner_col);
    default:
      return new Token(tokid, yytext, line, scanner_line, scanner_col);
  }
}

// NOT REENTRANT!!!!
int makeToken(int tokid) {
  yylval.token = createTokenRespectingSubclass(tokid);
  advance_cursor();
  return tokid;
}

