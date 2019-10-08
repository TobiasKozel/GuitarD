#pragma once
#include <map>
#include "src/graph/Node.h"

#define REGISTER_DEC_TYPE(NAME) \
    static DerivedRegister<NAME> reg

#define REGISTER_DEF_TYPE(NAME) \
    DerivedRegister<NAME> NAME::reg(#NAME)


template<typename T> Node* createT() { return new T; }

struct BaseFactory {
  typedef std::map<std::string, Node* (*)()> map_type;

  static Node* createInstance(std::string const& s) {
    map_type::iterator it = getMap()->find(s);
    if (it == getMap()->end())
      return 0;
    return it->second();
  }

protected:
  static map_type* getMap() {
    // never delete'ed. (exist until program termination)
    // because we can't guarantee correct destruction order 
    if (!map) { map = new map_type; }
    return map;
  }

private:
  static map_type* map;
};

template<typename T>
struct DerivedRegister : BaseFactory {
  DerivedRegister(std::string const& s) {
    getMap()->insert(std::make_pair(s, &createT<T>));
  }
};