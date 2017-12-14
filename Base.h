#if !defined(__Base_H__)

#define __Base_H__

#include <set>

class Base {
  public:
    virtual ~Base() {};
    virtual string toString() = 0;
    virtual bool hasPointerTo(Base *base, set<Base *> *seen);
};

#endif
