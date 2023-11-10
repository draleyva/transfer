/*CONTROLHEADER*/

#ifndef TOKENVAR_H
#define TOKENVAR_H

#define TOKEN_DATETIME_S "${DATETIME}"
#define TOKEN_DATETIME_N 0
#define TOKEN_DATE_S "${DATE}"
#define TOKEN_DATE_N 1
#define TOKEN_TIME_S "${TIME}"
#define TOKEN_TIME_N 2
#define TOKEN_COMPLETETIME_S "${COMPLETETIME}"
#define TOKEN_COMPLETETIME_N 3
#define TOKEN_COMPLETEDATE_S "${COMPLETEDATE}"
#define TOKEN_COMPLETEDATE_N 4
#define TOKEN_PID_S "${PID}"
#define TOKEN_PID_N 5
#define TOKEN_IPADDRESS_S "${IPADDRESS}"
#define TOKEN_IPADDRESS_N 6
#define TOKEN_COMMONPROGRAMFILES_S "${COMMONPROGRAMFILES}"
#define TOKEN_COMMONPROGRAMFILES_N 7
#define TOKEN_PROGRAMFILES_S "${PROGRAMFILES}"
#define TOKEN_PROGRAMFILES_N 8
#define TOKEN_TEMP_S "${TEMP}"
#define TOKEN_TEMP_N 9

/*Lista de parámetros*/
#define TOKEN_VAR1_S "${VAR1}"
#define TOKEN_VAR1_N 10
#define TOKEN_VAR2_S "${VAR2}"
#define TOKEN_VAR2_N 11
#define TOKEN_VAR3_S "${VAR3}"
#define TOKEN_VAR3_N 12
#define TOKEN_VAR4_S "${VAR4}"
#define TOKEN_VAR4_N 13
#define TOKEN_VAR5_S "${VAR5}"
#define TOKEN_VAR5_N 14
#define TOKEN_VAR6_S "${VAR6}"
#define TOKEN_VAR6_N 15
#define TOKEN_VAR7_S "${VAR7}"
#define TOKEN_VAR7_N 16
#define TOKEN_VAR8_S "${VAR8}"
#define TOKEN_VAR8_N 17
#define TOKEN_VAR9_S "${VAR9}"
#define TOKEN_VAR9_N 18
#define TOKEN_VAR10_S "${VAR10}"
#define TOKEN_VAR10_N 19

#define TOKEN_VAR_MAX (TOKEN_VAR10_N-TOKEN_VAR1_N+1)

#define TOKEN_VAR_FORMAT "${%s}"

#define TOKEN_MAX 20

#define TOKEN_SYMBOL_0 "$("
#define TOKEN_SYMBOL_1 "${"
#define TOKEN_VAR "VAR."

#define TOKEN_MAX_BUFFER 8192

#ifdef  __cplusplus
extern "C" {
#endif

class ToolToken
{
private:
  char m_szTokenVar[TOKEN_MAX][CD_STREAM_512B];
  char m_Parameter__[TOKEN_VAR_MAX][CD_STREAM_1K];
public:
  ToolToken();
  void resetvar();
  char* gettokval(int32_t index, char* tokval, int tokvalsize);
  void replacevartoken(int32_t index, const char* newtoken);
  int resolvetoken(char* in, int insize);
  void setvar(const char* token, char* s);
  void setvar(int32_t index, char* s);
  int istoken(char* in);
  int resolvetoken_s(char* in, int insize);
  static int replacetoken(const char* in, const char* token, char* value, char* working, int workingsize);
};

#ifdef  __cplusplus
}
#endif

#endif
