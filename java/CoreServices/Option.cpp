#include "CoreDefinition.h"
#include "ToolHeader.h"
#include "Tool.h"
#include <ace/Get_Opt.h>
#include "Option.h"

Option::Option(int32_t argc, char* argv[])
{
  for (int32_t i = 0; i < argc; ++i)
  {
    options.insert(OptionPair(i, argv[i]));
  }

  option = 0;
}

int32_t Option::add(const ACE_TCHAR* name, int short_option, ACE_Get_Opt::OPTION_ARG_MODE has_arg)
{
  char optionequal[CD_STREAM_1K];
  char* ptr;

  evaluate.insert(OptionPair(short_option, std::string(name)));

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4 && __GNUC_MINOR__ >= 5) || defined (WIN32)
  if (has_arg == ACE_Get_Opt::OPTION_ARG_MODE::ARG_OPTIONAL || has_arg == ACE_Get_Opt::OPTION_ARG_MODE::NO_ARG)
#else
  if ((int32_t)has_arg == 2 || (int32_t)has_arg == 0)
#endif
    return 0;

  sprintf_s(optionequal, sizeof(optionequal), "%s", name);

  for (OptionIterator oi = options.begin(); oi != options.end(); ++oi)
  {
    ptr = (char*)strstr(oi->second.c_str(), optionequal);
    if (ptr != nullptr)
    {
      char nextchar = *(ptr + strlen(optionequal));
      if (nextchar == 0 || nextchar == '=' || nextchar == ' ')
        return 0;
    }
  }

  return -1;
}

int32_t Option::operator ()(void)
{
  int32_t opt = -9999;
  char sopt[CD_STREAM_1K];
  char optionequal[CD_STREAM_1K];
  char* ptr;

  if (option == static_cast<int32_t>(options.size()))
    return EOF;

  ACE_OS::strcpy(sopt, options[option].c_str());

  for (OptionIterator oi = evaluate.begin(); oi != evaluate.end(); ++oi)
  {
    sprintf_s(optionequal, sizeof(optionequal), "%s", oi->second.c_str());
    ptr = strstr(sopt, optionequal);
    if (ptr != nullptr)
    {
      char* nextchar = ptr + strlen(optionequal);
      if (*nextchar == 0 || *nextchar == '=' || *nextchar == ' ')
      {
        opt = oi->first;
        ACE_OS::strcpy(argumentValue, nextchar + (*nextchar == '=' || *nextchar == ' ' ? 1 : 0));
        break;
      }
    }
  }

  ++option;

  return opt;
}

const char* Option::argument()
{
  return argumentValue;
}

void Option::reset()
{
  option = 0;
}
