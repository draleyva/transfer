#pragma once

#include <stdint.h>
#include <string>
#include "GenericArray.h"

class ByteArray : public GenericArray<uint8_t>
{
public:  
  ByteArray(int32_t l): GenericArray(l)
  {
  }

  const char* asConstChar()
  {
    return (const char*)array;
  }

  void assign2(std::string& out)
  {
    out.clear();
    out.assign((const char*)array, _size);
  }  
};

#define BYTEARRAY(_varname, _size) std::unique_ptr<ByteArray, FreeGenericArray<uint8_t>> _varname(new ByteArray(_size))
