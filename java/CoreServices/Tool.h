/*CONTROLHEADER*/
#pragma once

#ifndef TOOL_H
#define TOOL_H

#ifndef TOOL_NOLINE
#define TOOL_NOLINE 1
#define TOOL_NOTAG 2
#define TOOL_NOFILE 3
#define TOOL_NOSPACE 4
#define TOOL_NONUMBER 5
#define TOOL_NOMEMORY 6
#endif

#define ANALYSIS_MARKS 0x00
#define ANALYSIS_EXPECTONLYNUMBERS 0x10

#define DATETIME_FORMAT_DOT 0
#define DATETIME_FORMAT_NORMAL 1

// Defines

#if defined(_WIN32) || defined(_WIN64)
#define VARIANT_BOOL2BOOL(x) (x==VARIANT_TRUE?TRUE:FALSE)
#else
#define WINAPI
#endif

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef MINGW
using PLATFORM = enum
{
  WINNT,
  WIN2K_XP,
  WIN9X,
  UNKNOWN
};
#endif

#include "BaseCoreServices.h"

using PROCESSLINE = int(WINAPI *)(int32_t index, char* line, char* errormessage);

class Tool : public BaseCoreServices
{
public:
#if defined(_WIN32) || defined(_WIN64)
  static VARTYPE BSTR2VARTYPE(BSTR val);
  static void WINAPI writebinarydata(int32_t dir, unsigned char* data, int size);
  static uint64_t calculateTimeDifference(SYSTEMTIME systime);
  //static void GetProcessMemoryInfo(HANDLE hProcess, double &WorkingSetSize, double &PeakWorkingSetSize, double &PagefileUsage);
  static void GetSystemInformation(SYSTEM_INFO* psysteminfo);
  static bool isDirectory(const char* path);
  static double GetMemoryUsageRatio();
  //static bool CreateTempFileName(const char *path, const char *prefix, char *tempfile);
  static bool compareInteger(const char* pinta, const char* pintb);
  static int64_t dv(double val, int exp);
  static bool RequestConfigFile(const char* key, char* config, int nconfig, bool subdir = true);
  static bool RequestControlFile(const char* key, char* config, int nconfig, bool subdir = true);
  static bool RequestOffsetFile(const char* key, char* config, int nconfig, bool subdir = true);
  static bool RequestFile(const char* key, char* config, int nconfig, const char* filetype, bool subdir = true);
  static bool RequestLogonInfo(const char* key, char* domain, int ndomain, char* user, int nuser, char* password,
                               int npassword);
  // La función ahora es MT
  static char* buildpath(const char* newpath, const char* sfile, char* output, int outputsize);
  static bool isSoftwareInitialized(const char* key);
  static bool InCommandLine(const char* key);
#else
  static long dv(double val,int exp);
#endif
  enum BASE { DECIMAL = 10, HEXADECIMAL = 16 };

  static void dateTime2s(time_t n, char* s, int ssize);
  static void date2s(time_t n, char* s, int ssize);
  static void time2s(time_t n, char* s, int ssize);
  static uint64_t getLastModify(const char* filepath);
  static uint32_t getPID();
  static uint32_t getTID();
  static uint64_t getTickCount();
  static void sleep(long ms);

  static double pown(double d, int64_t exp);
  static int32_t getLastDay(const char* date);
  static int32_t between_date(int32_t format, const char* test, const char* start, const char* end);
  static int32_t between_time(int32_t format, const char* test, const char* start, const char* end);
  static int32_t getTempFileName(char* tempfile, int tempfilesize);
  static int32_t getTempPath(char* temppath, int temppathsize);
  static int32_t same_date(int32_t format, const char* test, const char* date);
  static int32_t string2RE(const char* n, const char* wildcard, char* re, int32_t resize);
  static double to_ndouble5(char* data, int l);
  static double to_ndouble2(char* data, int l);
  static int32_t getNextDate(char* date, int ndays);
  static int32_t getDay(const char* date);
  static int32_t getCurrentYearDay();
  static int32_t getMDay(const char* date);
  static int32_t getMonth(char* date);
  static bool isDate(const char* sdate);
  static bool isTime(const char* stime);
  static int32_t isDouble(const char* field);
  static int32_t isDouble(const char* field, int l);
  static int32_t isAlphanumeric(const char* field);
  static int32_t isAlphanumericSpecial(const char* field);
  static int32_t isAlphanumericWildcard(const char* field);
  static int32_t isHexadecimal(const char* field);

  static bool isNumeric(const uint8_t* field, int32_t length, BASE base = DECIMAL);
  static bool isNumeric(const uint8_t* field, BASE base = DECIMAL);
  static bool isNumeric(const char* field, BASE base = DECIMAL);
  static int32_t isNotEmpty(char* field, int l);

  static int32_t is_binary(unsigned char* pdata, size_t len, int analysis, ...);
  static int32_t getDay2(const char*);
  static int32_t parseBoolean(const char*);
  static int32_t terminateProcessGroup(uint32_t pgid, int signal = 9);
  static double to_double(const char* field);
  static double to_ndouble(const char* data, int len);
  static bool dateInRange(const char* range, const char* date);
  static bool timeInRange(const char* range, const char* hour);
  static int32_t toInt32(const char* field, int32_t l);
  static int32_t toInt32(const char* field);
  static int32_t toInt32Ext(const char* field, int32_t def = 0);
  static int64_t toInt64(const char* field, int32_t l);
  static int64_t toInt64(const char* field);
  static char* getSysTime();
  static void getSysDateTimeMT(char* datehour, int32_t dhsize);
  static void getSysTimeMT(char* outhour, int32_t ohsize);
  static char* getSysTimemsMT(char* outhour, int32_t ohsize);
  static char* getCompleteSysDate(char middle = '.');
  static char* getCompleteSysDateMT(char* szsysdate, char middle = '.');
  static char* getCompleteSysDateTimemsMT(char* sysdatehourms, int32_t sdhmssize);
  static char* getCompleteSysTime();
  static char* getCompleteSysTimeMT(char* szsyshour, int32_t szsyshoursize);
  static char* getSysDateTimemsMT(char* sysdatehourms, int32_t sdhmssize);
  static void getSysDateMT(char* outdate, int32_t odsize);
  static bool isPrintable(uint8_t* buffer, int64_t buffersize);
  static void replaceNotPrintable(uint8_t* buffer, int64_t buffersize, char newchar);
  static void date2String(time_t date, const char* separator, char* datestring);
  static void time2String(time_t time, const char* separator, char* timestring);
  static time_t string2Date(const char* date);
  static time_t string2Time(const char* time);
  static int toasciihex(unsigned char* buffer, int len, char* ascii);
  static time_t timeStamp2Time(const char* timestamp, bool separators);
  static time_t timeStamp2Time(const char* date, const char* time, bool separators);
  static void completeDateYear(const char* fec_mmdd, char* output, int32_t outputsize);
  static int getNextColon(const char* buffer, int index, char* output, int32_t outputsize);
  static int decode(const char* inp, const char* formula, char* output, int32_t outputsize);

  static int mkdir(const char* dir);
  static uint64_t calculateTimeDifference(const char* firsttime, time_t reftime = 0);
  static uint64_t calculateTimeDifference(bool includems, int format, const char* secondtime, const char* firsttime);
  static void seconds2DHMS(uint64_t totalseconds, int32_t& days, int32_t& hours, int32_t& minutes, int32_t& seconds);
  static char* get_sysmonthday();
  static double to_double2(char* field);
  static int buildtoken(char* ptr, int size);

  static int val(int32_t value);

  static int replacestr(char* str, size_t size, const char* ps1, const char* ps2);
  static double to_doublen(const char* data, int n);
  static int getLineS(char* file, int nline, char* sline, int len);
  static int getLineN(char* file, int nline, int* number);
  static int getFile(const char* file, bool binary, unsigned char* content, int& filesize);
  static int getFileSize(const char* file, int& filesize);
  static int readFileLine(const char* file, int nline, char* sline, int len);
  static int hexToData(unsigned char* in, int len, unsigned char* out);
  static int dataToHex(unsigned char* in, int len, unsigned char* out);
  static void toLower(char* str);
  static void toUpper(char* str);

  static bool wildcardMatch(const char* pstring, const char* pmatch);
  static void putToken(char* basebuff, int32_t basebuffsize, char* token, char* value);
  static int getToken(char* basebuff, char* token, char* value);
  static int getToken(const char* basebuff, const char* opentag, int32_t opentagsize, const char* closetag, int32_t closetagsize, char* value);
  static int getToken(const char* basebuff, int& offset, const char* opentag, int opentaglength, const char* closetag,
                      char* value);
  static void removeToken(char* basebuff, char* token);
  static int removechr(char* str, int f);
  static int normalizeCRLF(char* str);
  static void byteArray2UTF8(unsigned char* input, int64_t inputsize, unsigned char* output, int64_t& outputsize);
  static void UTF82byteArray(unsigned char* input, int64_t inputsize, unsigned char* output, int64_t& outputsize);
  static int32_t getUTF8Size(unsigned char* input, int64_t inputsize);
  static char* strnonblank(const char* buffer);
  static int parseFile(const char* filename, const char* tokens);
  static int append2File(char* filename, char* data, ...);
  static int bcd2ascii(unsigned char* bcd, int len, bool removef, char* ascii);
  static int ascii2bcd(char* ascii, unsigned char* bcd, int* len);
  static int bcd2ascii10(unsigned char* bcd, int len, char* ascii);

  static int hexa2ascii(unsigned char* bcd, int len, char* ascii);
  static int ascii2hexa(char* bcd, unsigned char* ascii, int* len);

  static void setcommandline(char*);
  static int getnexttoken(char* p, char* tok, int pos, char* value);
  static int hexString2Binary(const char* hexstring, int hexstringlength, unsigned char* binary, int& binarysize);
  static int binary2HexString(unsigned char* binary, int binarysize, char* hexstring, int& hexstringlength);
  static int byteArray2HexString(unsigned char* binary, int binarysize, char* hexstring, int& hexstringlength);
  static void hexString2String(const char* szInpBuffer, int iInpSize, char* szOutBuffer);
  static void string2HexString(const char* szInpBuffer, int iInpSize, char* szOutBuffer, int iOutSize);
  static void UnpackString(const char* szInpBuffer, int iInpSize, char* szOutBuffer);
  static int UnpackString(const char* szInpBuffer, int iInpSize);
  static int GetSeparatorPos(const char* szInpBuffer, int iMaxSizeToFind, int iSeparator);
  static bool copyFile(const char* source, const char* target);
  static int readFile(const char* filename, PROCESSLINE processline, int& count, char* errormessage = nullptr);
  static int32_t readTextFile(const char* filename, std::string& content);

  static void debugger(FILE* debug, const char* format, ...);
  static void debugger(FILE* debug, uint64_t tick, const char* format, ...);
  static void debuggerExt(FILE* debug, const char* format, ...);
  static void debugger(FILE* debug, uint32_t id, const char* format, ...);
  static void debuggerNVS(FILE* debug, uint32_t id, const char* buffer);
  static void debuggerExt(FILE* debug, char* workingbuffer, int32_t workingbuffersize, uint32_t id, const char* format, ...);
  static void output(FILE* debug, const char* format, ...);
  static int YYYYMMDD2DDMMYYYY(char* fechai, char* fechao, int fechaosize);
  static void getDatePart(const char* daterange, char* part1, int sizepart1, char* part2, int sizepart2);
  static void getTimePart(const char* hourrange, char* part1, int sizepart1, char* part2, int sizepart2);

  static int32_t getBoolean(FILE* fd, const char* key, bool& value);
  static int32_t getProperty(FILE* fd, const char* key, char* value);
  static int32_t getProperty(FILE* fd, const char* key, int& value);
  static int32_t getProperty(FILE* fd, const char* key, long& value);
  static int32_t setProperty(FILE* fd, const char* key, unsigned long value);
  static int32_t setProperty(FILE* fd, const char* key, const char* value);

  static int is_in_c(char c, char* valids);
  static int is_nemptyzero(char* campo, int l);
  static void fillWithZeros(int32_t iNumber, char* strBuffer, int iSize);
  static int32_t buildNumericToString(char* stream, int32_t streamsize, int32_t val, int32_t lengthTrama, int32_t option);
  static int buildFieldsEnable(char*, char*);
  static int32_t baseName(const char* filepath, char* filebasename, int32_t filebasenamesize);
  static void wchar2char(std::wstring wstr, char* output, int& outputsize);
  static void char2wchar(const char* inout, std::wstring& output);

  // replacements
  static int32_t setEnv(const char* name, const char* value);
  static int32_t getEnv(const char* name, char* value, int32_t valuesize);
  static int32_t unlink(const char* filename);
};

#define TOOL_DIFFTIME_MS 0
#define TOOL_DIFFTIME_S 1
#define TOOL_DIFFTIME_M 2

// función de prueba para cada una de las librerias
void Tool();

using BINARYWRITER = void(WINAPI *)(int32_t dir, unsigned char* data, int size);

#define VOIDCONFIGURATION "VOID"

#define XML_HEADER_UTF8 "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"

#if defined(_WIN32) || defined(_WIN64)

#define RETURNTHREAD(x) return x;
#define DECLARETHREAD(ThreadWorker) DWORD WINAPI ThreadWorker(LPVOID param)
#define DECLARETHREADATT(x)
#define CREATETHREAD(handle, x, Worker, parameter) handle = CreateThread(NULL, 0, Worker, parameter, 0, NULL);
#define DECLARETHREADHANDLE(x) HANDLE x;
#define DECLARETHREADHANDLES(x, y) HANDLE x[y];
#define DECLARETHREADHANDLES_DYNAMIC(x, y) HANDLE *x = new HANDLE[y];
#define TERMINATETHREAD(x) TerminateThread(x, 0); CloseHandle(x);
#define WAITFORTHREADS(x, y) WaitForMultipleObjects(y, x, TRUE, INFINITE);

#define APPLICATIONLOOP() \
MSG msg;\
while(GetMessage(&msg, 0, 0, 0) != 0)\
{\
  TranslateMessage(&msg);\
  DispatchMessage(&msg);\
}

int pipe(int32_t pipefd[2]);
int getppid();
int setpgid(int32_t p, int pid);
int fork();
int kill(int32_t pid, int killsignal);
int waitpid(int32_t pid, int* status, int mode);

#else

#define RETURNTHREAD(x) return x;
#define DECLARETHREAD(ThreadWorker) void *ThreadWorker(void *param)

#ifdef AIX
#define DECLARETHREADATT(x) pthread_attr_t x;\
pthread_attr_init(&x);\
pthread_attr_setdetachstate(&x, PTHREAD_CREATE_JOINABLE);\
pthread_attr_setstacksize(&x, PTHREAD_STACK_MIN+65536*2);
#else
#define DECLARETHREADATT(x) pthread_attr_t x;\
pthread_attr_init(&x);\
pthread_attr_setdetachstate (&x, PTHREAD_CREATE_JOINABLE);
#endif

#define CREATETHREAD(handle, x, Worker, parameter) pthread_create(&handle, &x, Worker, (void*) parameter);
#define DECLARETHREADHANDLE(x) pthread_t x;
#define DECLARETHREADHANDLES(x, y) pthread_t x[y];
#define DECLARETHREADHANDLES_DYNAMIC(x, y) pthread_t *x = new pthread_t[y];
#define TERMINATETHREAD(x) pthread_cancel(x);
#define WAITFORTHREADS(x, y) {\
  for(int32_t wt_ = 0; wt_ < y; wt_++)\
  {\
    pthread_join(x[wt_], NULL);\
  }\
}

#define APPLICATIONLOOP() \
while(getchar());

#endif

#define UNSIGNED(x, y) ((unsigned x)y)

#define HBVAL(X) ((X>>4) & 0xF)
#define LBVAL(X) (X & 0xF)

#define NEWSTR(var, length) var = new char [length+1]; memset(var, 0, length+1);
#define DELETESTR(var) var!=NULL?delete [] var, var = NULL:0;

/*
Si se procesa bcd los valores deben estar del 0 al 9
*/
#define CHEX2ASCII(X) (X>9?'A'+(X-10):'0'+X)
#define CASCII2HEX(X) (X>='a'?X-'a'+10:(X>='A'?X-'A'+10:X-'0'))
#define DATEDOTFORMAT_LENGTH 10
#define STANDARDDATE_LENGTH 8
#define COMPLETEDATETIMENOMS_LENGTH 14
#define COMPLETEDATETIMEMS_LENGTH 17

#define DECLARECHAR(x, s, t) char x[s]; strcpy_s(x, s, t);

#ifdef  __cplusplus
}
#endif

#endif
