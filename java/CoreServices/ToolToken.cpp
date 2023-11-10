/*CONTROLHEADER*/

#include "ToolHeader.h"
#include "ToolPlatform.h"
#include "CoreDefinition.h"
#include "ToolToken.h"
#include "Tool.h"

#ifdef  __cplusplus
extern "C" {
#endif

ToolToken::ToolToken()
{
  int index = 0;

  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_DATETIME_S/*0*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_DATE_S /*1*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_TIME_S /*2*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_COMPLETETIME_S /*3*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_COMPLETEDATE_S /*4*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_PID_S /*5*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_IPADDRESS_S /*11*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_COMMONPROGRAMFILES_S /*12*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_PROGRAMFILES_S /*13*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_TEMP_S /*14*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR1_S /*15*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR2_S /*16*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR3_S /*17*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR4_S /*18*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR5_S /*19*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR6_S /*20*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR7_S /*21*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR8_S /*22*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR9_S /*23*/);
  ACE_OS::strcpy(m_szTokenVar[index++], TOKEN_VAR10_S /*24*/);

  memset(m_Parameter__, 0, sizeof(m_Parameter__));
}

void ToolToken::resetvar()
{
  int i;
  int index = 0;

  for (i = TOKEN_VAR1_N; i <= TOKEN_VAR10_N; ++i)
    sprintf(m_szTokenVar[i], "$(VAR%d)", index++);
}

void ToolToken::replacevartoken(int32_t index, const char* newtoken)
{
  char value[CD_STREAM_100B];

  if (strstr(newtoken, TOKEN_SYMBOL_0) == nullptr && strstr(newtoken, TOKEN_SYMBOL_1) == nullptr)
    ACE_OS::sprintf(value, TOKEN_VAR_FORMAT, newtoken);
  else
    ACE_OS::strcpy(value, newtoken);

  ACE_OS::strcpy(m_szTokenVar[index + TOKEN_VAR1_N], value);
}

void ToolToken::setvar(const char* token, char* s)
{
  int i;

  for (i = TOKEN_VAR1_N; i <= TOKEN_VAR10_N; ++i)
  {
    if (strcmp(m_szTokenVar[i], token) == 0)
    {
      ACE_OS::strcpy(m_szTokenVar[i - TOKEN_VAR1_N], s);
      break;
    }
  }
}

void ToolToken::setvar(int32_t index, char* s)
{
  if (index >= TOKEN_VAR_MAX)
    return;

  memset(m_Parameter__[index], 0, CD_STREAM_1K);
  strcpy_s(m_Parameter__[index], sizeof(m_Parameter__[index]), s);
}

int ToolToken::resolvetoken(char* in, int insize)
{
  while (istoken(in))
    resolvetoken_s(in, insize);

  return 0;
}

// existe algun token?
int ToolToken::istoken(char* in)
{
  char* ptok[TOKEN_MAX];
  int i;
  int val = 0;

  if (strstr(in, TOKEN_SYMBOL_0) == nullptr && strstr(in, TOKEN_SYMBOL_1) == nullptr)
    return 0;

  for (i = 0; i < TOKEN_MAX; ++i)
  {
    ptok[i] = strstr(in, m_szTokenVar[i]);
    if (ptok[i] != nullptr)
      val++;
  }

  return val;
}

char* ToolToken::gettokval(int32_t index, char* tokval, int tokvalsize)
{
  ACE_UNUSED_ARG(tokvalsize);

  switch (index)
  {
  case TOKEN_DATETIME_N:
    Tool::getSysDateTimeMT(tokval, tokvalsize);
    break;
  case TOKEN_DATE_N:
    Tool::getSysDateMT(tokval, tokvalsize);
    break;
  case TOKEN_TIME_N:
    Tool::getSysTimeMT(tokval, tokvalsize);
    break;
  case TOKEN_COMPLETETIME_N:
    Tool::getCompleteSysTimeMT(tokval, tokvalsize);
    break;
  case TOKEN_COMPLETEDATE_N:
    Tool::getCompleteSysDateMT(tokval);
    break;
  case TOKEN_PID_N:
    sprintf_s(tokval, tokvalsize, "%u", Tool::getPID());
    break;
#if defined _WIN32 && !defined MINGW
  case TOKEN_COMMONPROGRAMFILES_N:
    SHGetFolderPath(nullptr,CSIDL_PROGRAM_FILES_COMMON, nullptr, 0, tokval);
    break;
  case TOKEN_PROGRAMFILES_N:
    SHGetFolderPath(nullptr,CSIDL_PROGRAM_FILES, nullptr, 0, tokval);
#endif
    break;
  case TOKEN_TEMP_N:
    {
      size_t requiredsize = 0;
      getenv_s(&requiredsize, tokval, tokvalsize, "TEMP");
    }
    break;
  case TOKEN_VAR1_N:
  case TOKEN_VAR2_N:
  case TOKEN_VAR3_N:
  case TOKEN_VAR4_N:
  case TOKEN_VAR5_N:
  case TOKEN_VAR6_N:
  case TOKEN_VAR7_N:
  case TOKEN_VAR8_N:
  case TOKEN_VAR9_N:
  case TOKEN_VAR10_N:
    strcpy_s(tokval, tokvalsize, m_Parameter__[index - TOKEN_VAR1_N]);
    break;
  }
  return tokval;
}

int ToolToken::resolvetoken_s(char* in, int insize)
{
  ACE_UNUSED_ARG(insize);

  int i;
  int len;
  int len1;
  int len2;
  char* pstart;
  char* tokval;
  char* ptok[TOKEN_MAX];
  char buffer[TOKEN_MAX_BUFFER];
  char* VAR__ = nullptr;

  if (strstr(in, TOKEN_SYMBOL_0) == nullptr && strstr(in, TOKEN_SYMBOL_1) == nullptr)
    return 0;

  pstart = in;

  VAR__ = new char[CD_STREAM_16K];

  for (i = 0; i < TOKEN_MAX; ++i)
  {
    ptok[i] = strstr(in, m_szTokenVar[i]);
    if (ptok[i] != nullptr)
    {
      memset(VAR__, 0, CD_STREAM_16K);
      len = static_cast<int>(strlen(m_szTokenVar[i]));

      len1 = static_cast<int>(ptok[i] - pstart);
      strncpy_s(VAR__, CD_STREAM_16K, pstart, len1);
      memset(buffer, 0, sizeof(buffer));

      tokval = gettokval(i, buffer, sizeof(buffer));
      len2 = static_cast<int>(strlen(tokval));

      strncpy_s(VAR__ + len1, CD_STREAM_16K - len1, tokval, len2);
      strcpy_s(VAR__ + len1 + len2, CD_STREAM_16K - (len1 + len2), ptok[i] + len);

      strcpy_s(in, insize, VAR__);
    }
  }

  delete [] VAR__;

  return 0;
}

int ToolToken::replacetoken(const char* in, const char* token, char* value, char* working, int workingsize)
{
  ACE_UNUSED_ARG(workingsize);

  char* tok;
  int len, len1, len2;
  char* auxin;

  auxin = new char [CD_STREAM_16K];

  strcpy_s(auxin, CD_STREAM_16K, in);

  tok = strstr(auxin, token);

  if (tok == nullptr)
  {
    strcpy_s(working, workingsize, in);

    delete [] auxin;
    return 0;
  }

  do
  {
    len = static_cast<int>(strlen(token));
    len1 = static_cast<int>(tok - auxin);

    memcpy(working, auxin, len1);

    len2 = static_cast<int>(strlen(value));
    memcpy(working + len1, value, len2);
    strcpy_s(working + len1 + len2, workingsize - (len1 + len2), tok + len);

    strcpy_s(auxin, CD_STREAM_16K, working);
  }
  while ((tok = strstr(auxin, token)) != nullptr);

  delete [] auxin;

  return 0;
}

#ifdef  __cplusplus
}
#endif
