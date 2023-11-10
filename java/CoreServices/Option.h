#pragma once

#include <map>
#include <set>
#include <list>

using OptionMap = std::map<int32_t, std::string>;
using OptionPair = std::pair<int32_t, std::string>;
using OptionIterator = std::map<int32_t, std::string>::iterator;

class Option
{
public:
  OptionMap options;
  OptionMap evaluate;
  int32_t option;
  char argumentValue[CD_STREAM_1K];
  Option(int32_t argc, char* argv[]);
  int32_t add(const ACE_TCHAR* name, int short_option, ACE_Get_Opt::OPTION_ARG_MODE has_arg);
  int32_t operator ()(void);
  const char* argument();
  void reset();
};
