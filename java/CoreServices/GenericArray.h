#pragma once

#include <stdint.h>
#include <string>

template<typename T>
class GenericArray
{
public:
  T* array;
  int32_t _limit;
  int32_t _size;

  GenericArray(int32_t l) : _size(l)
  {
    _limit = l;
    array = new T[l];
  }

  int32_t limit()
  {
    return _limit;
  }

  int32_t size()
  {
    return _size;
  }

  void reset()
  {
    _size = _limit;
  }

  virtual ~GenericArray()
  {
    delete[] array;
  }
};

namespace
{
  template<typename T>
  struct FreeGenericArray
  {
    void operator()(GenericArray<T>* p) { delete p; }
  };
}

#define GENERICARRAY(TYPE, _varname, _size) std::unique_ptr<GenericArray<TYPE>, FreeGenericArray<TYPE>> _varname(new GenericArray<TYPE>(_size))
