#if !defined(__TYPE_H__)
#define __TYPE_H__

#include "Base.h"
#include <set>

// The base class for types

class TyType: public Base {
 public:
  virtual string toString() = 0;
};

// The type for an expression that we haven't yet resolved

class TyUnresolvedType : public TyType {
 public:
  virtual string toString();
};

// A null - not used during parsing, only typechecking.

class TyNull : public TyType {
 public:
   virtual string toString();
};

// An int

class TyInt : public TyType {
 public:
  virtual string toString();
};

// A double

class TyDouble : public TyType {
 public:
  virtual string toString();
};

// A bool

class TyBool : public TyType {
 public:
  virtual string toString();
};

// A string

class TyString : public TyType {
 public:
  virtual string toString();
};

// A void (only used for function return types)

class TyVoid : public TyType {
 public:
  virtual string toString();
};

class TyNamedType : public TyUnresolvedType {
};

// A type name. Either a class or an interface

class TyName : public TyNamedType {
 public:
  TyName(string name) : Name(name) {};
  virtual string toString();
  string Name;
};

// An array type

class TyArray : public TyUnresolvedType {
 public:
  TyArray(TyType* type) : Type(type) {};
  virtual string toString();
  virtual bool hasPointerTo(Base *base, set<Base *> *seen);
  TyType* Type;
};

// An unknown type

class TyUnknown : public TyType {
 public:
  virtual string toString();
};

ostream& operator << (ostream& stream, TyType* type);

#endif
