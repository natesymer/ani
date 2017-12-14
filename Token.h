#if !defined(__TOKEN_H__)
#define __TOKEN_H__

#include <iostream>

using namespace std;

class Token {
 public:
  Token(int type, string text, string *lineText, int line, int column);
  virtual string toString();
  int line();
  int column();
  string text();
  int type();
  string lineText();
 protected:
  int		Type;
  string	Text;
  string*	LineText;
  int		Line;
  int		Column;
};

class TokString : public Token {
 public:
  TokString(string text, string *lineText, int line, int column);
  virtual string toString();
  string value();
 private:
  string Value;
};

class TokIdent : public Token {
 public:
  TokIdent(string text, string *lineText, int line, int column);
  virtual string toString();
  string name();
 private:
  string Name;
};

class TokInt : public Token {
 public:
  TokInt(string text, string *lineText, int line, int column, bool hex = false);
  virtual string toString();
  int value();
 private:
  int Value;
};

class TokDouble : public Token {
 public:
  TokDouble(string text, string *lineText, int line, int column);
  virtual string toString();
  double value();
 private:
  double Value;
};

class TokBool : public Token {
 public:
  TokBool(bool value, string *lineText, int line, int column);
  virtual string toString();
  bool value();
 private:
  bool Value;
};

ostream& operator << (ostream& stream, Token& token);

#endif
