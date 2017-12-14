#include <sstream>
#include <iostream>
#include <stdlib.h>
#include "Token.h"
#include "AST.h"
#include "TokenNumbers.h"

using namespace std;

static string map[300]; 

string tokenName(int token) {
  if (map[TokAND] != "TokAND") {
    for(size_t i = 256; i < 300; i++)
      map[i] = "<illegal>";
    map[TokAND] = "TokAND";
    map[TokBOOL] = "TokBOOL";
    map[TokBREAK] = "TokBREAK";
    map[TokBoolConst] = "TokBoolConst";
    map[TokCLASS] = "TokCLASS";
    map[TokDOUBLE] = "TokDOUBLE";
    map[TokDoubleConst] = "TokDoubleConst";
    map[TokELSE] = "TokELSE";
    map[TokEQUAL] = "TokEQUAL";
    map[TokEXTENDS] = "TokEXTENDS";
    map[TokFOR] = "TokFOR";
    map[TokGREATEREQUAL] = "TokGREATEREQUAL";
    map[TokID] = "TokID";
    map[TokIF] = "TokIF";
    map[TokIMPLEMENTS] = "TokIMPLEMENTS";
    map[TokINT] = "TokINT";
    map[TokINTERFACE] = "TokINTERFACE";
    map[TokIntConst] = "TokIntConst";
    map[TokLESSEQUAL] = "TokLESSEQUAL";
    map[TokNEW] = "TokNEW";
    map[TokNEWARRAY] = "TokNEWARRAY";
    map[TokNOTEQUAL] = "TokNOTEQUAL";
    map[TokNULL] = "TokNULL";
    map[TokOR] = "TokOR";
    map[TokPRINT] = "TokPRINT";
    map[TokREADINTEGER] = "TokREADINTEGER";
    map[TokREADLINE] = "TokREADLINE";
    map[TokRETURN] = "TokRETURN";
    map[TokSTRING] = "TokSTRING";
    map[TokStringConst] = "TokStringConst";
    map[TokTHIS] = "TokTHIS";
    map[TokTYPEID] = "TokTYPEID";
    map[TokVOID] = "TokVOID";
    map[TokWHILE] = "TokWHILE";
  }  

  if (token < 256)
    return string(1, token);

  return map[token];
}

bool ValidTokenType(int type) {
  return (' ' <= type && type <= '~') ||
    (256 <= type && type < 300 && map[type] != "<illegal>");
}

Token::Token(int ty, string txt, std::string *lineTxt, int lin, int col) {
  if (!ValidTokenType(ty)) {
    cerr << "Error: Illegal token type: " << ty << endl;
    exit(1);
  }

  Type = ty;
  Text = txt;
  Line = lin;
  LineText = lineTxt;
  Column = col;
}

int Token::line() {
  return Line;
}

int Token::column() {
  return Column;
}

string Token::text() {
  return Text;
}

int Token::type() {
  return Type;
}

string Token::lineText() {
  return *LineText;
}

TokString::TokString(string txt, string *lineTxt, int li, int col) :
  Token(TokStringConst, txt, lineTxt, li, col)
{
  Value = txt;
}

string TokString::toString() {
  stringstream ss;

  ss << "StringConst(text = \"" << Text << "\", line = " << Line << ", column = " << Column << ", line text = \"" << *LineText << "\")";

  return ss.str();
 }

string TokString::value() {
  return Value;
}

TokIdent::TokIdent(string txt, string *lineTxt, int li, int col) :
  Token(TokID, txt, lineTxt, li, col)
{
  Name = txt;
}

string TokIdent::toString() {
  stringstream ss;

  ss << "ID(" << Name << ", line = " << Line << ", column = " << Column << ", line text = \"" << *LineText << "\")";

  return ss.str();
}

string TokIdent::name() {
  return Name;
}

TokInt::TokInt(string txt, string *lineTxt, int li, int col, bool hex) :
  Token(TokIntConst, txt, lineTxt, li, col)
{
  stringstream ss;

  if (hex)
    ss << std::hex << txt;
  else
    ss << txt;
  ss >> Value;
}

string TokInt::toString() {
  stringstream ss;

  ss << "IntConst(" << Value << ", text = \"" << Text << "\", line = " << Line << ", column = " << Column << ", line text = \"" << *LineText << "\")";

  return ss.str();
}

int TokInt::value() {
  return Value;
}

TokDouble::TokDouble(string txt, string *lineTxt, int li, int col) :
  Token(TokDoubleConst, txt, lineTxt, li, col)
{
  stringstream ss(txt);
  ss >> Value;
}

string TokDouble::toString() {
  stringstream ss;

  ss << "DoubleConst(" << Value << ", text = \"" << Text << "\", " << Line
     << ", " << Column << ", \"" << *LineText << "\")";
  return ss.str();
}

double TokDouble::value() {
  return Value;
}

TokBool::TokBool(bool val, string *lineTxt, int li, int col) :
  Token(TokBoolConst, val ? "true" : "false", lineTxt, li, col)
{
  Value = val;
}

string TokBool::toString() {
  stringstream ss;

  if (Value)
    ss << "BoolConst(true";
  else
    ss << "BoolConst(false";

  ss << ", line = " << Line << ", column = " << Column << ", line text = \"" << *LineText << "\")";
  return ss.str();
}

bool TokBool::value() {
  return Value;
}

string Token::toString() {
  stringstream ss;

  if (!ValidTokenType(Type)) {
    cerr << "Error: Illegal token type: " << Type << endl;
    exit(1);
  }

  ss << "Token(" << "type = " << tokenName(Type) << ", text = \"" << Text 
     << "\", line = " << Line << ", column = " << Column << ", line text = \""
     << *LineText << "\")";
  return ss.str();
}

ostream& operator << (ostream& stream, Token& token) {
  stream << token.toString();
  return stream;
}
