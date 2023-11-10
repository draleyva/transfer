/*CONTROLHEADER*/

#define TOOL_LIBRARY

#include "ToolHeader.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>
#include <string.h>
#include "CoreDefinition.h"
#include "ToolPlatform.h"
#include "Tool.h"
#include "ToolToken.h"
#include "ByteArray.h"

char* g__commandline = nullptr;

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
#pragma comment (lib, "ACE")

void _INVALIDPARAMETERHANDLER_(const wchar_t* expression,
                               const wchar_t* function,
                               const wchar_t* file,
                               unsigned int line,
                               uintptr_t pReserved)
{
}

class _MAIN_
{
public:
  _MAIN_();
};

_MAIN_::_MAIN_()
{
  _set_invalid_parameter_handler(_INVALIDPARAMETERHANDLER_);
  _CrtSetReportMode(_CRT_ASSERT, 0);
}

static _MAIN_ _GLOBAL_;

#endif

uint64_t Tool::getTickCount()
{
#if defined(HAVE_GETTIMEOFDAY)
  timeval tp;
  ::gettimeofday(&tp, 0);

  return ((int64_t)tp.tv_sec * 1000) + (int64_t)(tp.tv_usec / 1000);
#elif defined(HAVE_FTIME)
  struct timeb tp;
  ftime(&tp);

  return (tp.time * 1000) + static_cast<int64_t>(tp.millitm);
#else
  return (int64_t)::time(0) * 1000;
#endif
}

void Tool::sleep(long millis)
{
  std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

double Tool::pown(double d, int64_t exp)
{
  return (pow(d, exp) + 1e-9);
}

#define B2VT_(x) if(strcmp(ptr,#x)==0)return x;
#define B2VT2_(x,y) if(strcmp(ptr,x)==0)return y;

#if defined(_WIN32) || defined(_WIN64)

#ifndef MINGW

bool Tool::isSoftwareInitialized(const char* key)
{
  char path[1024];
  char temp[1024];
  
  FILE* fh = nullptr;

  memset(path, 0, sizeof(path));

  if (SHGetFolderPath(nullptr,CSIDL_PROGRAM_FILES_COMMON, nullptr, 0, temp) == S_OK)
  {
    char sspath1[_MAX_PATH];
    char sspath2[_MAX_PATH];
    char sspath3[_MAX_PATH];
    char sspath4[_MAX_PATH];

    _splitpath_s(key, sspath1, sizeof(sspath1), sspath2, sizeof(sspath2), sspath3, sizeof(sspath3), sspath4,
                 sizeof(sspath4));

    sprintf_s(path, sizeof(path), "%s\\%s\\%s.installed", temp, key, sspath3);

    if (openFile(&fh, path, "r") == 0)
    {
      ::fclose(fh);
      return true;
    }

    if (openFile(&fh, path, "w") == 0)
      ::fclose(fh);
  }

  return false;
}

double Tool::GetMemoryUsageRatio()
{
  MEMORYSTATUSEX statex;

  statex.dwLength = sizeof (statex);

  GlobalMemoryStatusEx(&statex);

  return statex.dwMemoryLoad * 1.0f / 100L;
}


VARTYPE Tool::BSTR2VARTYPE(BSTR val)
{
  CHAR* ptr;

  USES_CONVERSION;

  ptr = W2A(val);

  B2VT2_("VT_BSTR", VT_BSTR)
  B2VT2_("VT_I4", VT_I4)

  return VT_EMPTY;
}

/*
void Tool::GetProcessMemoryInfo(HANDLE hProcess, double &WorkingSetSize, double &PeakWorkingSetSize, double &PagefileUsage)
{
  PROCESS_MEMORY_COUNTERS pmc;

  WorkingSetSize = 0;
  PeakWorkingSetSize = 0;
  PagefileUsage = 0;

  if ( ::GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
  {
    WorkingSetSize = pmc.WorkingSetSize/1024.0f;
    PeakWorkingSetSize = pmc.PeakWorkingSetSize /1024.0f;
    PagefileUsage = pmc.PagefileUsage /1024.0f;
  }
}*/

void Tool::GetSystemInformation(SYSTEM_INFO* psysteminfo)
{
  using LPFN_ISWOW64PROCESS = BOOL(WINAPI *)(HANDLE, PBOOL);
  using LPFN_GETNATIVESYSTEMINFO = void(WINAPI *)(SYSTEM_INFO*);
  LPFN_ISWOW64PROCESS fnIsWow64Process;  
  BOOL bIsWow64 = FALSE;

  fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");

  if (nullptr != fnIsWow64Process)
  {
    if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
      return;
    // Trabajar con la información
  }
  if (!bIsWow64)
  {
    GetSystemInfo(psysteminfo);
  }
  else
  {
    LPFN_GETNATIVESYSTEMINFO fnGetNativeSystemInfo = (LPFN_GETNATIVESYSTEMINFO)GetProcAddress(
      GetModuleHandle(TEXT("kernel32")), "GetNativeSystemInfo");
    if (fnGetNativeSystemInfo != nullptr)
      fnGetNativeSystemInfo(psysteminfo);
  }
}

bool Tool::RequestFile(const char* key, char* config, int nconfig, const char* filetype, bool subdir)
{
  char temp[1024];
  char sspath1[_MAX_PATH];
  char sspath2[_MAX_PATH];
  char sspath3[_MAX_PATH];
  char sspath4[_MAX_PATH];
  char directory[1024];
  FILE* fh = nullptr;

  memset(directory, 0, sizeof(directory));

  if (SHGetFolderPath(nullptr,CSIDL_PROGRAM_FILES_COMMON, nullptr, 0, temp) == S_OK)
  {
    if (subdir)
    {
      _splitpath_s(key, sspath1, sizeof(sspath1), sspath2, sizeof(sspath2), sspath3, sizeof(sspath3), sspath4,
                   sizeof(sspath4));
      sprintf_s(config, nconfig, "%s\\%s\\%s.%s", temp, key, sspath3, filetype);
      sprintf_s(directory, sizeof(directory), "%s\\%s", temp, key);
    }
    else
    {
      sprintf_s(config, nconfig, "%s\\%s.%s", temp, key, filetype);
    }

    if (openFile(&fh, config, "r") == 0)
    {
      ::fclose(fh);
      return true;
    }

    mkdir(directory);
  }

  return false;
}

bool Tool::RequestOffsetFile(const char* key, char* config, int nconfig, bool subdir)
{
  return RequestFile(key, config, nconfig, "offset", subdir);
}

bool Tool::RequestConfigFile(const char* key, char* config, int nconfig, bool subdir)
{
  return RequestFile(key, config, nconfig, "config", subdir);
}

bool Tool::RequestControlFile(const char* key, char* config, int nconfig, bool subdir)
{
  return RequestFile(key, config, nconfig, "control", subdir);
}

bool Tool::RequestLogonInfo(const char* key, char* domain, int ndomain, char* user, int nuser, char* password,
                            int npassword)
{
  char path[1024];
  char temp[1024];
  char sspath1[_MAX_PATH];
  char sspath2[_MAX_PATH];
  char sspath3[_MAX_PATH];
  char sspath4[_MAX_PATH];
  FILE* fh = nullptr;

  memset(path, 0, sizeof(path));

  if (SHGetFolderPath(nullptr,CSIDL_PROGRAM_FILES_COMMON, nullptr, 0, temp) == S_OK)
  {
    _splitpath_s(key, sspath1, sizeof(sspath1), sspath2, sizeof(sspath2), sspath3, sizeof(sspath3), sspath4,
                 sizeof(sspath4));

    sprintf_s(path, sizeof(path), "%s\\%s\\%s.logon", temp, key, sspath3);

    if (openFile(&fh, path, "r") == 0)
    {
      if (fgets(domain, ndomain, fh) == nullptr || fgets(user, nuser, fh) == nullptr || fgets(password, npassword, fh)
        == nullptr)
      {
        ::fclose(fh);
        return false;
      }

      removechr(domain, 0x0A);
      removechr(user, 0x0A);
      removechr(password, 0x0A);

      ::fclose(fh);
      return true;
    }
  }

  return false;
}

char* Tool::buildpath(const char* newpath, const char* sfile, char* output, int outputsize)
{
  //static char snewpath[1000];
  char buff[2][1024];
  _splitpath_s(sfile, nullptr, 0, nullptr, 0, buff[0], 1024, buff[1], 1024);
  sprintf_s(output, outputsize, "%s\\%s%s", newpath, buff[0], buff[1]);

  return output;
}

#endif

bool Tool::compareInteger(const char* pinta, const char* pintb)
{
  bool iret = false;
  __int64 inta, intb;

  if (pinta == nullptr || pintb == nullptr)
    return iret;

  if (isNumeric(pinta) && isNumeric(pintb))
  {
    inta = _atoi64(pinta);
    intb = _atoi64(pintb);
    iret = (inta == intb) ? true : false;
  }
  else
    iret = !strcmp(pinta, pintb) ? true : false;

  return iret;
}

bool Tool::InCommandLine(const char* key)
{
  char commandlinebuffer[2048];
  strcpy_s(commandlinebuffer, sizeof(commandlinebuffer),GetCommandLine());
  return strstr(commandlinebuffer, key) != nullptr;
}

//bool Tool::CreateTempFileName(const char *path, const char *prefix, char *tempfile)
//{
//  // Tomado de
//  // http://msdn.microsoft.com/en-us/library/aa363875(VS.85).aspx
//  DWORD dwRetVal = 0;
//  DWORD dwBufSize = _MAX_PATH;
//  UINT uRetVal = 0;
//  TCHAR lpPathBuffer[_MAX_PATH];
//
//  if(path == NULL)
//  {
//    // Get the temp path.
//    dwRetVal = GetTempPath(dwBufSize,     // length of the buffer
//                          lpPathBuffer); // buffer for pat
//
//    if (dwRetVal > dwBufSize || (dwRetVal == 0))
//      return false;
//  }
//  else
//    strcpy_s(lpPathBuffer, sizeof(lpPathBuffer), path);
//
//  // Create a temporary file.
//  uRetVal = GetTempFileName(lpPathBuffer, // directory for tmp files
//                            prefix,  // temp file name prefix
//                            0,            // create unique name
//                            tempfile);  // buffer for name
//  if (uRetVal == 0)
//    return false;
//
//  return true;
//}

void WINAPI Tool::writebinarydata(int32_t dir, unsigned char* data, int size)
{
  char buffer[100];
  size_t bsize;
  memset2(buffer);
  char currdir[256];
  char filename[CD_STREAM_1K];
  char currcmd[CD_STREAM_1K];
  char* pcurrcmd = nullptr;
  FILE* fh = nullptr;
  ACE_OS::putenv("TZ=");
  ACE_OS::tzset();

  memset2(currdir);

#if defined(_WIN32) || defined(_WIN64)
  ::GetCurrentDirectory(256, currdir);
#else
  strcpy(currdir,get_current_dir_name());
#endif

  pcurrcmd = g__commandline;
  if (pcurrcmd)
  {
    ACE_OS::strcpy(currcmd, pcurrcmd);
    replacechr(currcmd, 0x20, '@');
  }

  sprintf_s(filename, sizeof(filename), "%s\\%s.binary", currdir, (pcurrcmd ? currcmd : "generic"));

  if (openFile(&fh, filename, "ab+") != 0)
    return;

  char date[CD_STREAM_1K];

  getSysDateMT(date, sizeof(date));

#if defined(_WIN32) || defined(_WIN64)
  bsize = sprintf_s(buffer, sizeof(buffer), "[%04d][%s][%s][%06d][%06d][%06d]", dir, date, getSysTime(),
                    GetCurrentProcessId(), GetCurrentThreadId(), size);
#else
  bsize = sprintf_s(buffer, sizeof(buffer), "[%04d][%s][%s][%06d][%06d][%06d]",dir, date, Tool::get_systime(),
    getpid(),pthread_self(),size);
#endif
  //fseek(fh,0,SEEK_END);
  fwrite(buffer, 1, bsize, fh);
  if (data != nullptr)
    fwrite(data, 1, size, fh);
  fputc('\n', fh);
  fflush(fh);
  ::fclose(fh);
}

bool Tool::isDirectory(const char* path)
{
  return GetFileAttributes(path) == FILE_ATTRIBUTE_DIRECTORY;
}

#endif


void Tool()
{
}

int64_t Tool::dv(double val, int exp)
{
  int i = 0;
  int64_t pot = 1;
  for (i = 0; i < exp; ++i)
    pot *= 10;

  return static_cast<int64_t>(val *= pot);
}

int Tool::parseBoolean(const char* s)
{
  if (s == nullptr)
    return 0;

#if defined(_WIN32) || defined(_WIN64)
  if (*s == 1 || *s == '1' || *s == 'S' || *s == 's' || *s == 'y' || *s == 'Y' || _stricmp(s, "ON") == 0 ||
    _stricmp(s, "true") == 0)
#else
  if(*s==1 || *s=='1' || *s=='S' || *s=='s' || *s=='y' || *s=='Y' || strcmp(s,"ON")==0|| strcmp(s,"on")==0 || strcmp(s,"true")==0|| strcmp(s,"TRUE")==0)
#endif
    return 1;

  return 0;
}

int Tool::is_binary(unsigned char* pdata, size_t len, int analysis, ...)
{
  int binary = 0;
  size_t i;
  size_t j;
  size_t characters;
  va_list marker;
  int times;
  unsigned char* rec = nullptr;
  va_start(marker, analysis);

  switch (analysis)
  {
  case ANALYSIS_MARKS:
    {
      times = va_arg(marker, int);
      characters = va_arg(marker, int);
      int character[5];
      int counter = 0;
      int found = 0;

      for (i = 0; i < characters && i < 5UL; ++i)
        character[i] = va_arg(marker, int);

      rec = pdata;
      for (i = 0; i < len; ++i)
      {
        for (j = 0; j < characters; j++)
          if (*rec == character[j])
          {
            counter++;
            break;
          }
        if (counter == times)
          break;
        if (*rec < 32)
          ++found;
        ++rec;
      }
      binary = found > 0 ? 1 : 0;
    }
    break;
  case ANALYSIS_MARKS | ANALYSIS_EXPECTONLYNUMBERS:
    {
      times = va_arg(marker, int);
      characters = va_arg(marker, int);
      int character[5];
      int counter = 0;
      int found = 0;

      for (i = 0; i < characters && i < 5; ++i)
        character[i] = va_arg(marker, int);

      rec = pdata;
      for (i = 0; i < len; ++i)
      {
        for (j = 0; j < characters; j++)
          if (*rec == character[j])
          {
            counter++;
            break;
          }
        if (counter == times)
          break;
        if (*rec < 32 || !isdigit(*rec))
          ++found;
        ++rec;
      }
      binary = found > 0 ? 1 : 0;
    }
    break;
  }
  va_end(marker);

  return binary;
}

int32_t Tool::getProperty(FILE* fd, const char* param, char* value)
{
  int found = 0;
  size_t l;
  char* equal = nullptr;

  if (fd == nullptr)
    return -1;

  BYTEARRAY(buffer, CD_STREAM_16K);
  
  if (fseek(fd, 0, SEEK_SET) != 0)
    return -1;

  l = strlen(param);
  while (found == 0 && fgets((char*)buffer->array, CD_STREAM_16K, fd) != nullptr)
    if (strncmp((char*)buffer->array, param, l) == 0)
    {
      if (!(buffer->array[l] == ' ' || buffer->array[l] == '='))
        continue;

      equal = strchr((char*) & buffer->array[l], '=');
      if (equal != nullptr)
      {
        found = 1;
        ACE_OS::strcpy(value, equal + 1);
        removechr(value, 10);
        removechr(value, 13);
      }
    }

  return (found == 1 ? 0 : -2);
}

int32_t Tool::getBoolean(FILE* fd, const char* key, bool& value)
{
  char temp[CD_STREAM_256B];
  int32_t result;

  result = getProperty(fd, key, temp);

  if (result == 0)
    value = parseBoolean(temp);

  return result;
}

int32_t Tool::getProperty(FILE* fd, const char* param, long& value)
{
  char temp[CD_STREAM_256B];

  memset(temp, 0, sizeof(temp));
  if (getProperty(fd, param, temp) != 0)
    return -1;

  if (!isNumeric(temp))
    return -2;

  value = atol(temp);

  return 0;
}

int32_t Tool::getProperty(FILE* fd, const char* param, int& value)
{
  char temp[CD_STREAM_256B];

  if (getProperty(fd, param, temp) != 0)
    return -1;

  if (!isNumeric(temp))
    return -2;

  value = atoi(temp);

  return 0;
}

int32_t Tool::setProperty(FILE* fd, const char* param, unsigned long value)
{
  fprintf(fd, "%s=%lu\n", param, value);

  return 0;
}

int32_t Tool::setProperty(FILE* fd, const char* param, const char* value)
{
  fprintf(fd, "%s=%s\n", param, value);

  return 0;
}

uint32_t Tool::getTID()
{
#if defined(_WIN32) || defined(_WIN64)
  return GetCurrentThreadId();
#else
#ifndef AIX
  return syscall(SYS_gettid);
#else
  return 0;
#endif
#endif
}

uint32_t Tool::getPID()
{
#if defined(_WIN32) || defined(_WIN64)
  return GetCurrentProcessId();
#else
  return getpid();
#endif
}

int Tool::append2File(char* filename, char* data, ...)
{
  va_list arglist;
  FILE* fh = nullptr;
  char buffer[CD_STREAM_8K];

  va_start(arglist, data);
  vsnprintf(buffer, sizeof(buffer), data, arglist);
  va_end(arglist);

  if (openFile(&fh, filename, "a+") == 0)
  {
    fprintf(fh, "%s\n", buffer);
    fflush(fh);
    ::fclose(fh);
  }

  return 0;
}

#ifdef NONDELETE
bool Tool::checkProcess(uint32_t pid)
{
#if defined(_WIN32) || defined(_WIN64)
  HANDLE handle = OpenProcess(SYNCHRONIZE/*PROCESS_ALL_ACCESS*/, FALSE, pid);
  DWORD ret = 0;

  if(handle == NULL)
    return false;

  ret = WaitForSingleObject(handle, 0);

  CloseHandle(handle);

  return ret == WAIT_TIMEOUT;
#else
  char filename[CD_STREAM_1K];
  char buf[CD_STREAM_1K];
  int fd = 0;

#ifdef AIX
  // Revisión para AIX
  sprintf(filename, "/proc/%u/status", pid);
#else
  sprintf(filename, "/proc/%u/cmdline", pid);
#endif

  fd = open(filename, O_RDONLY);
  if (fd < 0)
  return (errno == EACCES);

  if (read(fd, buf, sizeof(buf)) <= 0)
  {
    close(fd);
    return false;
  }

  close(fd);

  return true;
#endif
}
#endif

int32_t Tool::readTextFile(const char* filename, std::string& content)
{
  std::ifstream istrm(filename);
  if (!istrm.is_open())
    return -1;

  std::ostringstream sstr;
  sstr << istrm.rdbuf();
  content.assign(sstr.str());

  return 0;
}

int Tool::readFile(const char* filename, PROCESSLINE processline, int& count, char* errormessage)
{
  FILE* fh = nullptr;
  char line[CD_STREAM_1K];
  int index = 0;
  int result = -1;

  if (openFile(&fh, filename, "r") != 0)
    return -1;

  while (fgets(line, CD_STREAM_1K, fh) != nullptr)
  {
    if (!(line[0] == '#' || (line[0] == '/' && line[1] == '/')))
    {
      removechr(line, 10);
      removechr(line, 13);
      trimlr(line, sizeof(line));

      if (line[0] != 0)
      {
        if ((result = processline(index, line, errormessage)) != 0)
          break;
        ++index;
      }
    }
  }

  ::fclose(fh);

  count = index;

  return result;
}

/*
int Tool::terminateProcess(uint32_t pid, int signal)
{
#if defined ACE_WIN32
  return ACE::terminate_process(pid);
#else
  return ACE_OS::kill(pid, signal);
#endif
}
*/

int Tool::terminateProcessGroup(uint32_t pgid, int signal)
{
#if defined(_WIN32) || defined(_WIN64)
  // Revisar esta implementación
  return 0;
#else
  return killpg(pgid, signal);
#endif
}

void Tool::debugger(FILE* debug, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_4K];
  char sztimebuffer[CD_STREAM_50B];

  va_start(arglist, format);
  vsprintf_s(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));

  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "[%s] %08u - %s\n", sztimebuffer, getTID(), szdebugger);
  fflush(debug);
}

void Tool::debugger(FILE* debug, uint64_t tick, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_4K];
  char sztimebuffer[CD_STREAM_50B];

  va_start(arglist, format);
  vsprintf_s(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));

  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "[%s] [T:%06" PRIu64 "] %s\n", sztimebuffer, tick, szdebugger);
  fflush(debug);
}

void Tool::debuggerExt(FILE* debug, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_32K];
  char sztimebuffer[CD_STREAM_50B];

  va_start(arglist, format);
  vsprintf_s(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));

  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "[%s] %08u - %s\n", sztimebuffer, getTID(), szdebugger);
  fflush(debug);
}

void Tool::debuggerNVS(FILE* debug, uint32_t id, const char* buffer)
{
  char sztimebuffer[CD_STREAM_50B];
  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));
  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "[%s] %08u - %s\n", sztimebuffer, id, buffer);
  fflush(debug);
}

void Tool::debugger(FILE* debug, uint32_t id, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_4K];
  char sztimebuffer[CD_STREAM_50B];

  va_start(arglist, format);
  vsprintf_s(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));
  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "[%s] %08u - %s\n", sztimebuffer, id, szdebugger);
  fflush(debug);
}

void Tool::debuggerExt(FILE* debug, char* workingbuffer, int32_t workingbuffersize, uint32_t id, const char* format, ...)
{
  va_list arglist;
  char sztimebuffer[CD_STREAM_50B];

  va_start(arglist, format);
  vsprintf_s(workingbuffer, workingbuffersize, format, arglist);
  va_end(arglist);

  getCompleteSysDateTimemsMT(sztimebuffer, sizeof(sztimebuffer));
  if (debug == nullptr)
  {
    fprintf(stdout, "[%s] %08u - %s\n", sztimebuffer, id, workingbuffer);
    fflush(stdout);
  }
  else
  {
    fprintf(debug, "[%s] %08u - %s\n", sztimebuffer, id, workingbuffer);
    fflush(debug);
  }
}

void Tool::output(FILE* debug, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_32K];

  va_start(arglist, format);
  vsprintf_s(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  if (debug == nullptr)
    debug = stdout;

  fprintf(debug, "%s\n", szdebugger);
  fflush(debug);
}

uint64_t Tool::getLastModify(const char* filepath)
{
  struct stat filestat;

  if (stat(filepath, &filestat) < 0)
    return 0;

  return filestat.st_mtime;
}

int32_t Tool::toInt32(const char* data)
{
  return toInt32(data, strlen(data));
}

int32_t Tool::toInt32(const char* data, int32_t l)
{
  char number[CD_STREAM_100B];

  if (!isNumeric((uint8_t*)data, l))
    return -1;

  sprintf(number, "%*.*s", l, l, data);

  return atoi(number);
}

int32_t Tool::toInt32Ext(const char* field, int32_t def)
{
  if (isNullOrEmpty(field))
    return def;

  return toInt32(field);
}

int64_t Tool::toInt64(const char* data)
{
  return toInt64(data, strlen(data));
}

int64_t Tool::toInt64(const char* data, int32_t l)
{
  char number[CD_STREAM_100B];

  if (!isNumeric((uint8_t*)data, l))
    return -1;

  sprintf(number, "%*.*s", l, l, data);

#if defined(_WIN32) || defined(_WIN64)
  return _atoi64(number);
#else
  return atoll(number);
#endif
}

double Tool::to_ndouble2(char* data, int l)
{
  int i;
  char x[80];

  for (i = 0; i < l; ++i)
    if (!isdigit(data[i]) && data[i] != ' ')
      return -1;

  sprintf_s(x, sizeof(x), "%*.*s", l, l, data);
  trim(x);
  return (atof(x) / 100.0f) + 1e-9;
}

double Tool::to_double2(char* data)
{
  return to_ndouble2(data, static_cast<int>(strlen(data)));
}

double Tool::to_ndouble5(char* data, int l)
{
  char x[80];

  if (!isNumeric((uint8_t*)data, l, DECIMAL))
    return -1;

  sprintf(x, "%*.*s", l, l, data);
  return atof(x) / 100000.0f + 1e-9;
}

double Tool::to_double(const char* data)
{
  if (!isDouble(data))
    return -1;

  return atof(data) + 1e-9;
}

double Tool::to_ndouble(const char* data, int len)
{
  char x[CD_STREAM_128B];

  if (!isDouble(data, len))
    return -1;

  memset(x, 0, sizeof(x));
  memcpy(x, data, len);

  return atof(x) + 1e-9;
}

double Tool::to_doublen(const char* data, int n)
{
  double decval = 1;
  int i;

  for (i = 0; i < n; ++i)
    decval *= 10;

  if (!isDouble(data))
    return -1;

  return atof(data) / decval;
}

/**************************************************************************/
/* Si option = 0: lo deja numericamente igual         DEF_NUMERIC_EQUAL   */
/*    option = 1: Rellena con zeros a  la izquierda   DEF_ZEROS_LEFT      */
/*    option = 2: Rellena con espacios a la derecha   DEF_SPACE_RIGHT     */
/*    option = 3: Rellena con espacios a la izquierda DEF_SPACE_LEFT      */
/**************************************************************************/

int32_t Tool::buildNumericToString(char* stream, int32_t streamsize, int32_t val, int32_t lengthTrama, int32_t option)
{
  int monto,
      tmpMonto,
      i = 0,
      tmpLength = 0;
  char* tmpTramaReturn = nullptr;

  if (val < 0)
  {
    sprintf_s(stream, streamsize, "%0*d", lengthTrama, 0);
    return 0;
  }

  tmpTramaReturn = new char[lengthTrama + 1];
  monto = val;
  tmpMonto = val;

  memset(tmpTramaReturn, 0, lengthTrama + 1);
  memset(stream, 0, lengthTrama + 1);

  do
  {
    i++;
    tmpMonto /= 10;
  }
  while (tmpMonto != 0);
  i -= 1;
  while (monto != 0)
  {
    tmpMonto = monto % 10;
    monto = (monto - tmpMonto) / 10;
    tmpTramaReturn[i] = tmpMonto + 48;
    i--;
  }
  tmpLength = strlen(tmpTramaReturn);
  if (option == 0)
    strcpy_s(stream, streamsize, tmpTramaReturn);
  else if (option == 1)
  {
    if (lengthTrama >= tmpLength)
    {
      memset(stream, '0', lengthTrama - tmpLength);
      memcpy(stream + (lengthTrama - tmpLength), tmpTramaReturn, tmpLength);
    }
    else
    {
      delete[] tmpTramaReturn;
      tmpTramaReturn = nullptr;
      return -1;
    }
  }
  else if (option == 2)
  {
    if (lengthTrama >= tmpLength)
    {
      memcpy(stream, tmpTramaReturn, tmpLength);
      memset(stream + tmpLength, ' ', lengthTrama - tmpLength);
    }
    else
    {
      delete[] tmpTramaReturn;
      tmpTramaReturn = nullptr;
      return -1;
    }
  }
  delete[] tmpTramaReturn;
  tmpTramaReturn = nullptr;
  return 0;
}

int Tool::buildFieldsEnable(char* msgFields, char* msgPBM)
{
  char cC = 0,
       cP = 0;
  int i = 0,
      j = 0,
      n = 0;

  while (msgPBM[i] != 0)
  {
    cC = msgPBM[i];
    if (cC >= 48 && cC <= 57)
      cC -= 48;
    else if (cC >= 65 && cC <= 90)
      cC -= 55;
    else if (cC >= 97 && cC <= 122)
      cC -= 87;

    cP = 0x08;
    for (j = 0; j < 4; j++)
    {
      if ((cC & cP) != 0)
        msgFields[n + j] = '1';
      else
        msgFields[n + j] = '0';
      cP >>= 1;
    }

    n += j;
    i++;
  }

  return 0;
}

/************************************************************/
/* FUNCIONES PARA VALIDACION                                */
/************************************************************/

int Tool::is_in_c(char c, char* valids)
{
  return (strchr(valids, c) == nullptr ? 0 : 1);
}

int Tool::isAlphanumeric(const char* field)
{
  if (isNullOrEmpty(field))
    return 0;

  const char* temp = field;

  while (*temp)
  {
    if (!isalnum(*temp))
      return 0;

    temp++;
  }

  return 1;
}

int Tool::isAlphanumericSpecial(const char* field)
{
  if (isNullOrEmpty(field))
    return 0;

  const char* temp = field;

  while (*temp)
  {
    if (!isalnum(*temp) && !ispunct(*temp))
      return 0;

    temp++;
  }

  return 1;
}

int Tool::isAlphanumericWildcard(const char* field)
{
  if (isNullOrEmpty(field))
    return 0;

  const char* temp = field;

  while (*temp)
  {
    if (!isalnum(*temp) && *temp != '*')
      return 0;

    temp++;
  }

  return 1;
}

int Tool::is_nemptyzero(char* campo, int l)
{
  if (isNullOrEmpty(campo))
    return 0;

  int i;

  for (i = 0; i < l; ++i)
    if (campo[i] != ' ' && campo[i] != 0x00 && campo[i] != '0')
      return 0;
  return 1;
}

void Tool::hexString2String(const char* szInpBuffer, int iInpSize, char* szOutBuffer)
{
  for (int32_t i = 0; i < iInpSize; i++, ++i)
  {
    char chTemp = ((szInpBuffer[i] > 64) ? szInpBuffer[i] - 0x41 + 10 : szInpBuffer[i] - 0x30) * 16 +
      ((szInpBuffer[i + 1] > 64) ? szInpBuffer[i + 1] - 0x41 + 10 : szInpBuffer[i + 1] - 0x30);
    szOutBuffer[i / 2] = chTemp;
  }
}

void Tool::string2HexString(const char* szInpBuffer, int iInpSize, char* szOutBuffer, int iOutSize)
{
  int size = (iInpSize >= iOutSize) ? iOutSize - 1 : iInpSize;

  for (int i = 0; i < size; ++i)
  {
    char hb = (0x0F & (szInpBuffer[i] >> 4));
    char lb = 0x0F & szInpBuffer[i];
    szOutBuffer[2 * i] = hb > 9 ? 0x37 + hb : hb + 0x30;
    szOutBuffer[2 * i + 1] = lb > 9 ? 0x37 + lb : lb + 0x30;
  }
}

void Tool::UnpackString(const char* szInpBuffer, int iInpSize, char* szOutBuffer)
{
  int i;
  for (i = 0; i < iInpSize; ++i)
  {
    szOutBuffer[2 * i] = ((szInpBuffer[i] >> 4) & 0x0F) + 0x30;
    szOutBuffer[2 * i] = (szOutBuffer[2 * i] == 0x3F) ? 'F' : ((szOutBuffer[2 * i] == 0x3D) ? 'D' : szOutBuffer[2 * i]);
    szOutBuffer[2 * i + 1] = ((szInpBuffer[i]) & 0x0F) + 0x30;
    szOutBuffer[2 * i + 1] = (szOutBuffer[2 * i + 1] == 0x3D) ? 'D' : szOutBuffer[2 * i + 1];
  }

  if (szOutBuffer[0] == 'F')
  {
    for (i = 0; i < 2 * iInpSize - 1; ++i)
      szOutBuffer[i] = szOutBuffer[i + 1];
    szOutBuffer[2 * iInpSize - 1] = 0;
  }
}

int Tool::UnpackString(const char* szInpBuffer, int iInpSize)
{
  auto szTemp = new char[iInpSize * 2 + 1];
  int iTempRes;
  bool bNegative = false, bImparDigits = false;

  memset(szTemp, 0, iInpSize * 2 + 1);

  UnpackString(szInpBuffer, iInpSize, szTemp);

  if (szTemp[0] == 'F')
    bImparDigits = true;

  if (szTemp[0] == 'D' || szTemp[1] == 'D')
    bNegative = true;

  iTempRes = bImparDigits ? (atoi(szTemp + (bNegative ? 2 : 1))) : (atoi(szTemp + (bNegative ? 1 : 0)));
  delete [] szTemp;
  return (bNegative ? -1 : 1) * iTempRes;
}

int Tool::GetSeparatorPos(const char* szInpBuffer, int iMaxSizeToFind, int iSeparator)
{
  int iRes;
  for (iRes = 0; iRes < iMaxSizeToFind; iRes++)
    if (szInpBuffer[iRes] == iSeparator)
      break;
  return (iRes == iMaxSizeToFind) ? S_FAIL : iRes;
}

char* Tool::strnonblank(const char* buffer)
{
  int pos = 0;
  char* retval = nullptr;

  if (*buffer != 0x20)
    return (char*)buffer;

  while (*(buffer + pos) != 0)
  {
    if (*(buffer + pos) != 0x20)
    {
      retval = (char*)buffer + pos;
      break;
    }

    ++pos;
  }

  return retval;
}

int Tool::getTempFileName(char* tempfile, int tempfilesize)
{
#if defined(_WIN32) || defined(_WIN64)
  DWORD dwRetVal;
  UINT uRetVal = 0;
  char temppath[CD_STREAM_2K];
  dwRetVal = GetTempPath(sizeof(temppath), // length of the buffer
                         temppath); // buffer for path
  if (dwRetVal > sizeof(temppath) || (dwRetVal == 0))
    return -1;

  uRetVal = GetTempFileName(temppath, // directory for tmp files
                            "temp__", // temp file name prefix
                            0, // create unique name
                            tempfile); // buffer for name

  if (uRetVal == 0)
    return -2;
#else
  ACE_UNUSED_ARG(tempfile);
  ACE_UNUSED_ARG(tempfilesize);
#endif
  return 0;
}

int Tool::getTempPath(char* temppath, int temppathsize)
{
#if defined(_WIN32) || defined(_WIN64)  
  UINT dwRetVal = GetTempPath(temppathsize, // length of the buffer
                         temppath); // buffer for path
  if (dwRetVal > temppathsize || dwRetVal == 0)
    return -1;

#else
  ACE_UNUSED_ARG(temppath);
  ACE_UNUSED_ARG(temppathsize);
#endif
  return 0;
}

// Luego se implementarás más funciones
#define TOOL_TOKENREPLACENEXTLINE "@replacenextline"

int Tool::parseFile(const char* filename, const char* tokens)
{
  char tempfile[CD_STREAM_1K];
  char linebuffer[CD_STREAM_8K];
  char workingbuffer[CD_STREAM_8K];
  int result = 0;
  FILE* ftempfile;
  FILE* ffile;
  int offset;
  int parseline = false;
  ToolToken token;
  char auxbuffer[CD_STREAM_8K];
  int tokensize = 0;
  char* ptrtok;
  char* context;
  char* separator = nullptr;
  char auxtokens[CD_STREAM_1K];
  char tokenbuffer[CD_STREAM_512B];
  char valuebuffer[CD_STREAM_512B];
  int tokenindex = 0;

  if (filename == nullptr)
    return -1;

  if (!isFile(filename))
    return -2;

  strcpy_s(auxtokens, sizeof(auxtokens), tokens);
  ptrtok = ACE_OS::strtok_r(auxtokens, "|", &context);

  while (ptrtok != nullptr)
  {
    separator = strchr(ptrtok, '=');
    if (separator != nullptr)
    {
      memset(tokenbuffer, 0, sizeof(tokenbuffer));
      memcpy(tokenbuffer, ptrtok, separator - ptrtok);
      strcpy_s(valuebuffer, sizeof(valuebuffer), separator + 1);
      token.replacevartoken(tokenindex, tokenbuffer);
      token.setvar(tokenindex, valuebuffer);
      ++tokenindex;
    }

    ptrtok = ACE_OS::strtok_r(nullptr, "|", &context);
  }

  sprintf_s(tempfile, sizeof(tempfile), "%s.temp", filename);

  if (openFile(&ftempfile, tempfile, "w") != 0)
    return -1;

  if (openFile(&ffile, filename, "r") != 0)
    return -1;

  memset(linebuffer, 0, sizeof(linebuffer));
  while (fgets(linebuffer, CD_STREAM_8K, ffile) != nullptr)
  {
    strcpy_s(workingbuffer, sizeof(workingbuffer), linebuffer);

    trimlr(workingbuffer, sizeof(workingbuffer));
    removechr(workingbuffer, 0x09);

    offset = 0;
    switch (parseline)
    {
    case 0:
      ptrtok = strnonblank(workingbuffer);
      if (ptrtok != nullptr)
      {
        if (memcmp(ptrtok, "//", 2) == 0)
        {
          offset = 2 + (ptrtok - workingbuffer);
        }
        else if (memcmp(ptrtok, "<!-", 3) == 0)
        {
          offset = 3 + (ptrtok - workingbuffer);
        }
      }

    // preparar la cadena que reemplazará a la siguiente
      if (offset > 0)
      {
        if (strncmp(workingbuffer + offset, TOOL_TOKENREPLACENEXTLINE,
                    tokensize = strlen(TOOL_TOKENREPLACENEXTLINE)) == 0)
        {
          strcpy_s(auxbuffer, sizeof(auxbuffer), workingbuffer + offset + tokensize + 1);
          result = token.resolvetoken(auxbuffer, sizeof(auxbuffer));
          parseline = 1;
        }
      }
      else
      {
        // No está definido un comando especial
        strcpy_s(auxbuffer, sizeof(auxbuffer),  linebuffer);
        result = token.resolvetoken(auxbuffer, sizeof(auxbuffer));
        strcpy_s(linebuffer, sizeof(linebuffer), auxbuffer);
      }

      fprintf(ftempfile, "%s", linebuffer);
      break;
    case 1:
      ptrtok = strnonblank(linebuffer);
      if (ptrtok != nullptr)
      {
        ACE_OS::strcpy(ptrtok, auxbuffer);
        ptrtok = linebuffer;
      }
      else
        ptrtok = auxbuffer;

      fprintf(ftempfile, "%s", ptrtok);
      parseline = 0;
      break;
    }
  }

  ::fclose(ftempfile);
  ::fclose(ffile);

  result = remove(filename);
  if (result != 0)
    return errno;

  result = rename(tempfile, filename);
  if (result != 0)
    return errno;

  return result;
}

bool Tool::isDate(const char* sdate)
{
  int m;
  int d;
  int y;

  y = toInt32(sdate, 4);
  m = toInt32(sdate + 4, 2);
  d = toInt32(sdate + 6, 2);

  if (! (1582 <= y))
    return false;
  if (! (1 <= m && m <= 12))
    return false;
  if (! (1 <= d && d <= 31))
    return false;
  if ((d == 31) && (m == 2 || m == 4 || m == 6 || m == 9 || m == 11))
    return false;
  if ((d == 30) && (m == 2))
    return false;
  if ((m == 2) && (d == 29) && (y % 4 != 0))
    return false;
  if ((m == 2) && (d == 29) && (y % 400 == 0))
    return true;
  if ((m == 2) && (d == 29) && (y % 100 == 0))
    return false;
  if ((m == 2) && (d == 29) && (y % 4 == 0))
    return true;

  return true;
}

bool Tool::isTime(const char* stime)
{
  int h;
  int m;
  int s;

  h = toInt32(stime, 2);
  m = toInt32(stime + 2, 2);
  s = toInt32(stime + 4, 2);

  if (!(h >= 0 && h <= 23))
    return false;
  if (!(m >= 0 && m <= 59))
    return false;

  if (!(s >= 0 && s <= 59))
    return false;

  return true;
}

int Tool::isDouble(const char* field, int l)
{
  int i;

  for (i = 0; i < l; ++i)
    if (field[i] < 0 || (!isdigit(field[i]) && (!(field[i] == '.' || (field[i] == '-' && i == 0)))))
      return 0;

  return l == 0 ? 0 : 1;
}

bool Tool::isNumeric(const uint8_t* field, int32_t l, BASE base)
{
  auto temp = (uint8_t*)field;
  int length = l;

  switch (base)
  {
  case DECIMAL:
    while (*temp && length > 0)
    {
      if (!isdigit(*temp) && (!(*temp == '-' && temp == field)))
        return false;

      temp++;
      --length;
    }
    break;
  case HEXADECIMAL:
    while (*temp && length > 0)
    {
      if (!isxdigit(*temp) && (!(*temp == '-' && temp == field)))
        return false;

      temp++;
      --length;
    }
    break;
  default:
    return false;
    break;
  }

  return l == 0 ? 0 : 1;
}

bool Tool::isNumeric(const uint8_t* field, BASE base)
{
  return isNumeric(field, strlen((char*)field), base);
}

bool Tool::isNumeric(const char* field, BASE base)
{
  return isNumeric((uint8_t*)field, base);
}

int Tool::isHexadecimal(const char* field)
{
  int i;
  int length = strlen(field);

  for (i = 0; i < length; ++i)
    if (!((field[i] >= '0' && field[i] <= '9') || (field[i] >= 'A' && field[i] <= 'F')))
      return 0;

  return length == 0 ? 0 : 1;
}

int Tool::isDouble(const char* field)
{
  return isDouble(field, static_cast<int>(strlen(field)));
}

void Tool::toLower(char* str)
{
  size_t i;
  for (i = 0; i < strlen(str); ++i)
    str[i] = tolower(str[i]);
}

void Tool::toUpper(char* str)
{
  size_t i;
  for (i = 0; i < strlen(str); ++i)
    str[i] = toupper(str[i]);
}


int Tool::isNotEmpty(char* buffer, int32_t buffersize)
{
  string str;
  str.append(buffer, buffersize);

  return str.find_first_not_of("\t\n ") == string::npos;
}

int Tool::getnexttoken(char* p, char* tok, int pos, char* value)
{
  char* pp = nullptr;
  char* dup = nullptr;
  char* context = nullptr;
  if (!p)
    return 0;

  dup = ACE_OS::strdup(p);

  pp = ACE_OS::strtok_r(dup, tok, &context);
  while (pos--)
    pp = ACE_OS::strtok_r(nullptr, tok, &context);

  if (pp)
    ACE_OS::strcpy(value, pp);

  free(dup);
  // Código de error
  return pos * (-1);
}

void Tool::setcommandline(char* comandline)
{
  if (g__commandline != nullptr)
    free(g__commandline);

  g__commandline = ACE_OS::strdup(comandline);
}

int Tool::getLineS(char* file, int nline, char* sline, int len)
{
  FILE* fh_ = nullptr;
  char* result;
  char temp[CD_STREAM_8K];
  int count = 0;
  int comment = 0;

  if (openFile(&fh_, file, "r") != 0)
    return TOOL_NOFILE;

  memset(sline, 0, len);

  while (count <= nline)
  {
    memset(temp, 0,CD_STREAM_8K);

    result = fgets(temp,CD_STREAM_8K, fh_);
    if (result != nullptr)
    {
      comment = 0;

      if (temp[0] == '#' || (temp[0] == '/' && temp[1] == '/'))
        comment = 1;

      if (count == nline && !comment)
      {
        char *sign1, *sign2;
        int auxlen;
        sign1 = strchr(temp, '<');
        if (sign1 == nullptr)
        {
          ::fclose(fh_);
          return TOOL_NOTAG;
        }

        sign2 = strchr(temp, '>');
        if (sign2 == nullptr)
        {
          ::fclose(fh_);
          return TOOL_NOTAG;
        }

        if (sign2 < sign1)
        {
          ::fclose(fh_);
          return TOOL_NOTAG;
        }

        auxlen = static_cast<int>(sign2 - sign1 - 1);
        if (auxlen > len - 1)
        {
          fclose(fh_);
          return TOOL_NOSPACE;
        }

        if (auxlen > 0)
          memcpy(sline, sign1 + 1, auxlen);

        break;
      }
      if (!comment)
        ++count;
    }
    else
    {
      if (feof(fh_))
      {
        fclose(fh_);
        return TOOL_NOLINE;
      }
    }
  }

  fclose(fh_);

  return 0;
}

//////////////////////////////////////////////////////////////////////////
//    WildcardMatch
//        pszString    - Input string to match
//        pszMatch    - Match mask that may contain wildcards like ? and *
//
//        A ? sign matches any character, except an empty string.
//        A * sign matches any string inclusive an empty string.
//        Characters are compared caseless.

bool Tool::wildcardMatch(const char* pszString, const char* pszMatch)
{
  // We have a special case where string is empty ("") and the mask is "*".
  // We need to handle this too. So we can't test on !*pszString here.
  // The loop breaks when the match string is exhausted.
  while (*pszMatch)
  {
    // Single wildcard character
    if (*pszMatch == '?')
    {
      // Matches any character except empty string
      if (!*pszString)
        return false;

      // OK next
      ++pszString;
      ++pszMatch;
    }
    else if (*pszMatch == '*')
    {
      // Need to do some tricks.

      // 1. The wildcard * is ignored.
      //    So just an empty string matches. This is done by recursion.
      //      Because we eat one character from the match string, the
      //      recursion will stop.
      if (wildcardMatch(pszString, pszMatch + 1))
        // we have a match and the * replaces no other character
        return true;

      // 2. Chance we eat the next character and try it again, with a
      //    wildcard * match. This is done by recursion. Because we eat
      //      one character from the string, the recursion will stop.
      if (*pszString && wildcardMatch(pszString + 1, pszMatch))
        return true;

      // Nothing worked with this wildcard.
      return false;
    }
    else
    {
      // Standard compare of 2 chars. Note that *pszSring might be 0
      // here, but then we never get a match on *pszMask that has always
      // a value while inside this loop.
      if (toupper(*pszString++) != toupper(*pszMatch++))
        return false;
    }
  }

  // Have a match? Only if both are at the end...
  return !*pszString && !*pszMatch;
}

int Tool::getLineN(char* file, int nline, int* number)
{
  int result = 0;
  char temp[CD_STREAM_8K];
  result = getLineS(file, nline, temp,CD_STREAM_8K);
  if (result != 0)
    return result;

  if (!isNumeric(temp))
    return TOOL_NONUMBER;

  *number = toInt32(temp);

  return 0;
}

/*cargar el contenido de todo el archivo*/

int Tool::getFile(const char* file, bool binary, unsigned char* content, int& filesize)
{
  int auxlen = 0;
  unsigned char rchar;
  int nchar = 0;
  FILE* fh_ = nullptr;

  if (openFile(&fh_, file, binary ? "rb" : "r") != 0)
    return TOOL_NOFILE;

  while (!feof(fh_))
  {
    if (auxlen >= filesize)
      break;

    if (binary)
    {
      if (fread(&rchar, 1, 1, fh_) != 1)
        break;
    }
    else
    {
      if ((nchar = fgetc(fh_)) == EOF)
        break;
      rchar = nchar;
    }

    content[auxlen++] = rchar;
  }

  fclose(fh_);

  if (auxlen > filesize)
    return TOOL_NOSPACE;

  filesize = auxlen;

  return 0;
}

int Tool::getFileSize(const char* file, int& filesize)
{
  FILE* fh_ = nullptr;

  if (openFile(&fh_, file, "rb") != 0)
    return TOOL_NOFILE;

  fseek(fh_, 0L, SEEK_END);
  filesize = ftell(fh_);
  fclose(fh_);

  return 0;
}

int Tool::buildtoken(char* ptr, int size)
{
  ACE_UNUSED_ARG(size);
  char temp[CD_STREAM_8K];

  memset(temp, 0, sizeof(temp));
  strcpy_s(temp, sizeof(temp), ptr);
  /*construye la cadena de salida con los id de la cadena de entrada*/
  /*devuelve el mismo texto por el momento.. hasta ser implementado*/
  return 0;
}

int Tool::readFileLine(const char* file, int nline, char* sline, int len)
{
  FILE* fh_ = nullptr;
  char* result;
  char temp[CD_STREAM_8K];
  int count = 0;
  int comment = 0;

  if (openFile(&fh_, file, "r") != 0)
    return TOOL_NOFILE;

  memset(sline, 0, len);

  while (count <= nline)
  {
    memset(temp, 0,CD_STREAM_8K);

    result = fgets(temp,CD_STREAM_8K, fh_);
    if (result != nullptr)
    {
      comment = 0;

      if (temp[0] == '#' || (temp[0] == '/' && temp[1] == '/'))
        comment = 1;

      if (count == nline && !comment)
      {
        /*se quita el cambio de linea*/
        if (temp[strlen(temp) - 1] == 10 || temp[strlen(temp) - 1] == 13)
          memcpy(sline, temp, strlen(temp) - 1);
        else
          memcpy(sline, temp, strlen(temp));

        break;
      }
      if (!comment)
        ++count;
    }
    else
    {
      if (feof(fh_))
      {
        fclose(fh_);
        return TOOL_NOLINE;
      }
    }
  }

  fclose(fh_);

  return 0;
}


int Tool::dataToHex(unsigned char* in, int len, unsigned char* out)
{
  int i;

  for (i = len - 1; i >= 0; i--)
  {
    out[2 * i] = (in[i] / 16) < 10 ? (in[i] / 16) + '0' : (in[i] / 16) - 10 + 'A';
    out[2 * i + 1] = (in[i] % 16) < 10 ? (in[i] % 16) + '0' : (in[i] % 16) - 10 + 'A';
  }

  out[2 * len] = 0;

  return 2 * len;
}

int Tool::val(int32_t value)
{
  if ('A' <= value && value <= 'F')
    value = value - 'A' + 10;
  else
    value = value - '0';
  return value;
}

int Tool::hexToData(unsigned char* in, int len, unsigned char* out)
{
  int i;
  unsigned char* aux = nullptr;
  int locallen = len;

  if ((len % 2) != 0)
  {
    aux = new unsigned char[len + 1];
    memcpy(aux, in, len);
    aux[len] = '0';
    ++locallen;
  }
  else
    aux = in;

  for (i = 0; i < locallen / 2; ++i)
    out[i] = val(aux[2 * i]) * 16 + val(aux[2 * i + 1]);

  if (locallen != len)
    delete [] aux;

  return locallen / 2;
}

int32_t Tool::string2RE(const char* n, const char* wildcard, char* re, int32_t resize)
{
  char* rec;

  std::string reresult;
  std::string wc = wildcard;

  for (rec = (char*)n; *rec != 0; rec++)
  {
    if (wc.find(*rec, 0) != std::string::npos)
    {
      if (ACE_OS::ace_isdigit(*rec))
      {
        reresult.append("[0-9]");
      }
      if (ACE_OS::ace_isalpha(*rec))
      {
        reresult.append("[0-9A-Za-z]");
      }
      else if (*rec == '*')
      {
        reresult.append(".*");
      }
      else if (*rec == '?')
      {
        reresult.append(".");
      }
    }
    else
      reresult.push_back(*rec);
  }

  strcpy_s(re, resize, reresult.c_str());

  return 0;
}

void Tool::putToken(char* basebuff, int32_t basebuffsize, char* token, char* value)
{
  char starttoken[200];
  char endtoken[200];
  sprintf_s(starttoken, sizeof(starttoken), "<%s>", token);
  sprintf_s(endtoken, sizeof(endtoken), "</%s>", token);

  /*Buscar token anterior y reemplazarlo*/
  removeToken(basebuff, token);

  std::stringstream ss;

  ss << starttoken << value << endtoken;

  strcpy_s(basebuff, basebuffsize, ss.str().c_str());
}

int Tool::getNextColon(const char* buffer, int index, char* output, int32_t outputsize)
{
  int i;
  char* ptr = nullptr;
  char* nextptr = nullptr;
  char* lastptr = nullptr;

  ptr = (char*)buffer;
  nextptr = (char*)buffer;

  for (i = 0; i <= index; ++i)
  {
    lastptr = nextptr;
    ptr = strchr(nextptr, ',');

    if (ptr == nullptr)
      break;
    nextptr = ptr + 1;
  }

  if (i < index)
    return 1;

  if (ptr == nullptr && lastptr != nullptr)
    strcpy_s(output, outputsize, lastptr);
  else
  {
    if (ptr > lastptr)
      memcpy(output, lastptr, ptr - lastptr);
    else
      return 1;
  }

  return 0;
}

int Tool::decode(const char* inp, const char* formula, char* output, int32_t outputsize)
{
  char tempdiscard[CD_STREAM_1K];
  char element[CD_STREAM_1K];
  char lastsaved[CD_STREAM_1K];
  int result = 0;
  int pair = 0;
  int found = 0;
  int count = 1;

  memset(tempdiscard, 0, sizeof(tempdiscard));
  getNextColon(formula, 0, tempdiscard, sizeof(tempdiscard));

  do
  {
    memset(element, 0, sizeof(element));
    result = getNextColon(formula, count, element, sizeof(element));
    ++count;
    if (!result)
    {
      switch (pair)
      {
      case 0:
        {
          pair = 1;
          if (strcmp(inp, element) == 0)
            found = 1;
        }
        break;
      case 1:
        {
          pair = 0;
          if (found) /*se ha encontrado el valor...*/
          {
            /*en element esta el valor que se ha encontrado para reemplazar */
            if (strcmp(element, "x") == 0 || strcmp(element, "X") == 0)
              strcpy_s(output, outputsize, inp);
            else
              strcpy_s(output, outputsize, element);

            return 0;
          }
        }
        break;
      }
      memset(lastsaved, 0, sizeof(lastsaved));
      strcpy_s(lastsaved, sizeof(lastsaved), element);
    }
  }
  while (!result);

  /*evaluar las posibilidades en el caso de error*/
  if (pair != 1)
  {
    strcpy_s(output, outputsize, inp);
    return 0;
  }

  /*no se encontro el valor de reemplazo, pero puede ser el valor x default*/
  if (strcmp(lastsaved, "x") == 0 || strcmp(lastsaved, "X") == 0)
    strcpy_s(output, outputsize,  inp);
  else
    strcpy_s(output, outputsize, lastsaved);

  return 0;
}

int Tool::getToken(char* basebuff, char* token, char* value)
{
  char starttoken[200];
  char endtoken[200];
  int start_length = 0;
  char* start = nullptr;
  char* end = nullptr;

  sprintf_s(starttoken, sizeof(starttoken), "<%s>", token);
  sprintf_s(endtoken, sizeof(endtoken), "</%s>", token);

  value[0] = 0;

  if (basebuff == nullptr)
    return -1;

  start = strstr(basebuff, starttoken);
  if (start == nullptr)
    return -2;

  end = strstr(basebuff, endtoken);
  if (end == nullptr)
    return -3;

  start_length = static_cast<int>(strlen(starttoken));

  memcpy(value, start + start_length, end - (start + start_length));
  value[end - (start + start_length)] = 0;

  return 0;
}

int Tool::getToken(const char* basebuff, const char* opentag, int32_t opentagsize, const char* closetag, int32_t closetagsize, char* value)
{
  size_t start_length = 0;
  
  value[0] = 0;

  if (basebuff == nullptr)
    return -1;

  const char* start = strstr(basebuff, opentag);
  if (start == nullptr)
    return -2;

  const char* end = strstr(basebuff, closetag);
  if (end == nullptr)
    return -3;

  start_length = strnlen_s(opentag, opentagsize);

  memcpy(value, start + start_length, end - (start + start_length));
  value[end - (start + start_length)] = 0;

  return 0;
}

int Tool::getToken(const char* basebuff, int& offset, const char* opentag, int opentaglength, const char* closetag,
                   char* value)
{
  char* start = nullptr;
  char* end = nullptr;
  int length = 0;

  value[0] = 0;

  if (basebuff == nullptr)
    return -1;

  start = (char*)strstr(basebuff + offset, opentag);
  if (start == nullptr)
    return -2;

  end = (char*)strstr(basebuff + offset, closetag);
  if (end == nullptr)
    return -3;

  memcpy(value, start + opentaglength, length = (end - (start + opentaglength)));
  offset = (end - start) + length + offset + 1;
  value[length] = 0;

  return 0;
}

int Tool::toasciihex(unsigned char* buffer, int len, char* ascii)
{
  int i;
  char hb, lb;

  for (i = 0; i < len; ++i)
  {
    hb = ((buffer[i] >> 4) & 0xF);
    lb = (buffer[i] & 0xF);
    *(ascii + 2 * i) = hb > 9 ? 0x37 + hb : hb + 0x30;
    *(ascii + 2 * i + 1) = lb > 9 ? 0x37 + lb : lb + 0x30;
  }
  ascii[2 * len] = 0;

  return 2 * len;
}

void Tool::removeToken(char* basebuff, char* token)
{
  char starttoken[200];
  char endtoken[200];
  int end_length = 0;
  char* start = nullptr;
  char* end = nullptr;
  int length = 0;
  sprintf_s(starttoken, sizeof(starttoken), "<%s>", token);
  sprintf_s(endtoken, sizeof(endtoken), "</%s>", token);

  if (basebuff == nullptr)
    return;

  start = strstr(basebuff, starttoken);
  if (start == nullptr)
    return;

  end = strstr(basebuff, endtoken);
  if (end == nullptr)
    return;

  end_length = static_cast<int>(strlen(endtoken));

  length = static_cast<int>(strlen(basebuff));

  memcpy(start, end + end_length, length - (end + end_length - start));
}


// size deberá de ser el sizeof de str, esto porque la cadena puede crecers
int Tool::replacestr(char* pstr, size_t size, const char* ps1, const char* ps2)
{
  string strMsg;

  if (pstr == nullptr || ps1 == nullptr || ps2 == nullptr || size <= 0)
    return 0;

  // Comprobar si existe la cadena a reemplazar
  // Si la cadena no existe no se hace cambios y no se reporta error
  if (strstr(pstr, ps1) == nullptr)
    return 0;

  // El error se reporta
  strMsg = pstr;
  strMsg.replace(strMsg.find(ps1), strlen(ps1), ps2);
  if (strMsg.size() > size)
    return -1;

  strcpy_s(pstr, size, strMsg.c_str());
  return 0;
}

int Tool::removechr(char* str, int f)
{
  char* p = str;
  char* sp = nullptr;

  /*
  Se realiza esta validación debido a que strchr está optimizada y permitirá una ejecución
  rápida en el caso que no exista el caracter.
  */
  if (strchr(str, f) == nullptr)
    return 1;

  while (*p)
  {
    if (*p == f)
    {
      sp = p;
      while (*sp)
      {
        *sp = *(sp + 1);
        ++sp;
      }
    }
    else
      ++p;
  }
  return 0;
}

int Tool::normalizeCRLF(char* str)
{
  int crcount = 0;

  replacechr(str, 0x0D, '.', crcount);
  if (crcount > 0)
  {
    removechr(str, 0x0A);
  }
  else
  {
    replacechr(str, 0x0A, '.');
  }

  return 0;
}

int Tool::ascii2bcd(char* ascii, unsigned char* bcd, int* len)
{
  int i, size, start = 0;
  char* pascii = nullptr;
  // ascii es un valor ascii bcd

  if ((size = static_cast<int>(strlen(ascii))) % 2)
  {
    bcd[start] = 0xF0;
    bcd[start++] += CASCII2HEX(*ascii);
  }
  pascii = ascii + start;
  for (i = 0; i < size / 2; i++, pascii += 2)
    bcd[i + start] = CASCII2HEX(*pascii) * 16 + CASCII2HEX(*(pascii+1));

  *len = (size + start) / 2;

  return 0;
}

void Tool::fillWithZeros(int32_t iNumber, char* strBuffer, int iSize)
{
  sprintf(strBuffer, "%0*d", iSize, iNumber);
}

int Tool::bcd2ascii(unsigned char* bcd, int len, bool removef, char* ascii)
{
  int i;
  //0102-1023
  if (len <= 0)
    return 0;

  for (i = 0; i < len; ++i)
  {
    ascii[2 * i] = CHEX2ASCII(HBVAL(bcd[i]));
    ascii[2 * i + 1] = CHEX2ASCII(LBVAL(bcd[i]));
  }
  ascii[2 * len] = 0;

  if (removef)
    removechr(ascii, 70/*F*/);

  return static_cast<int>(strlen(ascii));
}

int Tool::bcd2ascii10(unsigned char* bcd, int len, char* ascii)
{
  int i;
  //0102-1023
  if (len <= 0)
    return 0;

  for (i = 0; i < len; ++i)
  {
    // si bcd[0] = 68 -> ASCII = "68"
    ascii[2 * i] = CHEX2ASCII((bcd[i]/10));
    ascii[2 * i + 1] = CHEX2ASCII((bcd[i]%10));
  }
  ascii[2 * len] = 0;
  removechr(ascii, 70/*F*/);
  return static_cast<int>(strlen(ascii));
}

int Tool::binary2HexString(unsigned char* binary, int binarysize, char* hexstring, int& hexstringlength)
{
  int idx = 0;
  int aux = 0;

  if (binary == nullptr || binarysize < 0 || hexstring == nullptr)
    return -1;

  hexstringlength = binarysize * 2;

  for (idx = 0; idx < binarysize; idx++)
  {
    aux = (binary[idx] >> 4) & 0x0F;
    hexstring[2 * idx] = (aux > 9 ? 0x37 : 0x30) + aux;
    aux = binary[idx] & 0x0F;
    hexstring[2 * idx + 1] = (aux > 9 ? 0x37 : 0x30) + aux;
  }

  return 0;
}

int Tool::byteArray2HexString(unsigned char* binary, int binarysize, char* hexstring, int& hexstringlength)
{
  int idx = 0;
  int aux = 0;

  if (binary == nullptr || binarysize < 0 || hexstring == nullptr)
    return -1;

  hexstringlength = binarysize * 2;

  for (idx = 0; idx < binarysize; idx++)
  {
    aux = (binary[idx] >> 4) & 0x0F;
    hexstring[2 * idx] = (aux > 9 ? 0x57 : 0x30) + aux;
    aux = binary[idx] & 0x0F;
    hexstring[2 * idx + 1] = (aux > 9 ? 0x57 : 0x30) + aux;
  }

  return 0;
}

int Tool::hexString2Binary(const char* hexstring, int hexstringlength, unsigned char* binary, int& binarysize)
{
  int i;
  if (hexstring == nullptr || binary == nullptr || hexstringlength < 0)
    return -1;

  binarysize = hexstringlength / 2;

  for (i = 0; i < binarysize; ++i)
    binary[i] = CASCII2HEX(hexstring[2*i]) << 4 | CASCII2HEX(hexstring[2*i+1]);

  return 0;
}

int Tool::ascii2hexa(char* ascii, unsigned char* bcd, int* len)
{
  int i, size, start = 0;
  char* pascii = nullptr;
  // ascii es un valor ascii bcd

  if ((size = static_cast<int>(strlen(ascii))) % 2)
    bcd[start++] = CASCII2HEX(*ascii);

  pascii = ascii + start;
  for (i = 0; i < size / 2; i++, pascii += 2)
    bcd[i + start] = CASCII2HEX(*pascii) * 16 + CASCII2HEX(*(pascii+1));

  *len = (size + start) / 2;

  return 0;
}

int32_t Tool::setEnv(const char* name, const char* value)
{
#if defined(_WIN32) || defined(_WIN64)
  return SetEnvironmentVariable(name, value) ? 0 : -1;
#else
  return ACE_OS::setenv(name, value, 1);
#endif
}

//extern char**environ;

int32_t Tool::getEnv(const char* name, char* value, int32_t valuesize)
{
#if defined(_WIN32) || defined(_WIN64)
  int result = GetEnvironmentVariable(name, value, valuesize);

  if (result == 0)
  {
    return -1;
  }

  if (result > valuesize)
    return -2;
#else
  ACE_UNUSED_ARG(valuesize);

  char *env = NULL;
  env = ::getenv(name);
  if(env == NULL)
    return -1;

  strcpy(value, env);
#endif

  return 0;
}

int32_t Tool::unlink(const char* filename)
{
#if defined(_WIN32) || defined(_WIN64)
  return _unlink(filename);
#else
  return ::unlink(filename);
#endif
}


void Tool::wchar2char(std::wstring wstr, char* output, int& outputsize)
{
  std::string strValue;
  strValue.assign(wstr.begin(), wstr.end());

  outputsize = sprintf(output, "%s", strValue.c_str());
}

void Tool::char2wchar(const char* inout, std::wstring& output)
{
  std::string strValue = inout;
  std::wstring wsTmp(strValue.begin(), strValue.end());

  output = wsTmp;
}

int32_t Tool::baseName(const char* filepath, char* filebasename, int32_t filebasenamesize)
{
  char copyfn[CD_STREAM_4K];
  strcpy_s(copyfn, sizeof(copyfn), filepath);
  replacechr(copyfn, '\\', '/');

  strcpy_s(filebasename, filebasenamesize, ACE::basename(copyfn, '/'));
  /*
#if defined(_WIN32) || defined(_WIN64)
  char drive[CD_STREAM_50B];
  char dir[CD_STREAM_50B];
  char extension[CD_STREAM_100B];

  _splitpath(filepath, drive, dir, filebasename, extension);
#else
*/
  char* ptr = nullptr;
  //strcpy(filebasename, basename(filepath));
  // Check again
  ptr = ACE_OS::strrchr(filebasename, '.');
  if (ptr != nullptr)
    *ptr = 0;

  return 0;
}

int Tool::hexa2ascii(unsigned char* bcd, int len, char* ascii)
{
  int i;
  //0102-1023
  if (len <= 0)
    return 0;

  for (i = 0; i < len / 2; ++i)
  {
    ascii[i] = CASCII2HEX(bcd[2*i]) * 16 + CASCII2HEX(bcd[2*i+1]);
  }
  ascii[len / 2] = 0;

  return len / 2;
}

bool Tool::copyFile(const char* source, const char* target)
{
  FILE* fhsource = nullptr;
  FILE* fhtarget = nullptr;
  char readbyte = 0;
  BOOL retval = FALSE;

  ACE_UNUSED_ARG(target);

#if defined(_WIN32) || defined(_WIN64)
  fhsource = _fsopen(source, "rb", _SH_DENYNO);
#else
  fhsource = fopen(source, "rb");
#endif
  if (fhsource)
  {
#if defined(_WIN32) || defined(_WIN64)
    fhtarget = _fsopen(source, "wb", _SH_DENYNO);
#else
    fhtarget = fopen(source, "wb");
#endif
    if (fhtarget)
    {
      while (!feof(fhsource))
      {
        if (fread(&readbyte, 1, 1, fhsource) < 1)
          break;
        if (fwrite(&readbyte, 1, 1, fhtarget) < 1)
          break;
      }
      if (feof(fhsource))
        retval = TRUE;
      fclose(fhtarget);
    }

    fclose(fhsource);
  }

  return retval;
}

int Tool::YYYYMMDD2DDMMYYYY(char* fechai, char* fechao, int fechaosize)
{
  ACE_UNUSED_ARG(fechaosize);

  char y[5];
  char m[3];
  char d[3];

  memset(y, 0, sizeof(y));
  memset(m, 0, sizeof(m));
  memset(d, 0, sizeof(d));

  memcpy(y, fechai, 4);
  memcpy(m, fechai + 4, 2);
  memcpy(d, fechai + 6, 2);

  sprintf_s(fechao, fechaosize, "%s%s%s", d, m, y);

  return 0;
}

void Tool::getDatePart(const char* daterange, char* part1, int sizepart1, char* part2, int sizepart2)
{
  ACE_UNUSED_ARG(sizepart1);
  ACE_UNUSED_ARG(sizepart2);

  const char* ptr = nullptr;

  ptr = strchr(daterange,GLOBALSEPARATOR_1);
  if (ptr == nullptr)
    return;

  // copiar cadenas
  strncpy_s(part1, sizepart1, daterange,GLOBALDATEYYYYMMDDLENGTH);
  strncpy_s(part2, sizepart2, ptr + 1,GLOBALDATEYYYYMMDDLENGTH);
}

void Tool::getTimePart(const char* hourrange, char* part1, int sizepart1, char* part2, int sizepart2)
{
  ACE_UNUSED_ARG(sizepart1);
  ACE_UNUSED_ARG(sizepart2);

  const char* ptr = nullptr;

  ptr = strchr(hourrange,GLOBALSEPARATOR_1);
  if (ptr == nullptr)
    return;

  // copiar cadenas
  strncpy_s(part1, sizepart1, hourrange,GLOBALDATEHHMMSSLENGTH);
  strncpy_s(part2, sizepart2, ptr + 1,GLOBALDATEHHMMSSLENGTH);
}

bool Tool::timeInRange(const char* range, const char* hour)
{
  char* ptr;

  char part1[GLOBALDATEHHMMSSLENGTH + 1];
  char part2[GLOBALDATEHHMMSSLENGTH + 1];
  char temprange[CD_STREAM_1K + 1];
  char* pContext = nullptr;

  // buscar por ;
  ptr = (char*)strchr(range,GLOBALSEPARATOR_0);
  if (ptr == nullptr)
  {
    // buscar por -
    ptr = (char*)strchr(range,GLOBALSEPARATOR_1);
    if (ptr == nullptr)
    {
      // comparación directa con la fecha
      if (strcmp(range, hour) == 0)
        return true;
    }
    else
    {
      // revisar rangos
      memset2(part1);
      memset2(part2);
      getTimePart(range, part1, sizeof(part1), part2, sizeof(part2));
      // Validar la fecha
      if (strcmp(part1, hour) <= 0 && strcmp(part2, hour) >= 0)
        return true;
    }
  }
  else
  {
    char* tempptr;
    memset2(temprange);
    strncpy_s(temprange, sizeof(temprange), range,CD_STREAM_1K);
    ptr = ACE_OS::strtok_r(temprange, GLOBALSEPARATOR_0_S, &pContext);

    while (ptr != nullptr)
    {
      tempptr = strchr(ptr,GLOBALSEPARATOR_1);
      if (tempptr == nullptr)
      {
        // comparación directa con la fecha
        if (strcmp(ptr, hour) == 0)
          return true;
      }
      else
      {
        // revisar rangos
        memset2(part1);
        memset2(part2);
        getTimePart(ptr, part1, sizeof(part1), part2, sizeof(part2));
        // Validar la hora
        if (strcmp(part1, hour) <= 0 && strcmp(part2, hour) >= 0)
          return true;
      }
      ptr = ACE_OS::strtok_r(nullptr, GLOBALSEPARATOR_0_S, &pContext);
    }
  }

  return false;
}

bool Tool::dateInRange(const char* range, const char* date)
{
  char* ptr;
  char* tempptr;
  char part1[GLOBALDATEYYYYMMDDLENGTH + 1];
  char part2[GLOBALDATEYYYYMMDDLENGTH + 1];
  char temprange[CD_STREAM_1K + 1];
  char* pContext = nullptr;

  // buscar por ;
  ptr = (char*)strchr(range,GLOBALSEPARATOR_0);
  if (ptr == nullptr)
  {
    // buscar por -
    ptr = (char*)strchr(range,GLOBALSEPARATOR_1);
    if (ptr == nullptr)
    {
      // comparación directa con la fecha
      if (strcmp(range, date) == 0)
        return true;
    }
    else
    {
      // revisar rangos
      memset2(part1);
      memset2(part2);
      getDatePart(range, part1, sizeof(part1), part2, sizeof(part2));
      // Validar la fecha
      if (strcmp(part1, date) <= 0 && strcmp(part2, date) >= 0)
        return true;
    }
  }
  else
  {
    memset2(temprange);
    strncpy_s(temprange, sizeof(temprange), range,CD_STREAM_1K);
    ptr = ACE_OS::strtok_r(temprange, GLOBALSEPARATOR_0_S, &pContext);

    while (ptr != nullptr)
    {
      tempptr = strchr(ptr,GLOBALSEPARATOR_1);
      if (tempptr == nullptr)
      {
        // comparación directa con la fecha
        if (strcmp(ptr, date) == 0)
          return true;
      }
      else
      {
        // revisar rangos
        memset2(part1);
        memset2(part2);
        getDatePart(ptr, part1, sizeof(part1), part2, sizeof(part2));
        // Validar la fecha
        if (strcmp(part1, date) <= 0 && strcmp(part2, date) >= 0)
          return true;
      }
      ptr = ACE_OS::strtok_r(nullptr, GLOBALSEPARATOR_0_S, &pContext);
    }
  }

  return false;
}

int Tool::getCurrentYearDay()
{
  time_t t;
  struct tm newtime;

  t = time(nullptr);
  localtime_s(&newtime, &t);

  return newtime.tm_yday;
}

int Tool::getDay(const char* date)
{
  time_t t1;
  struct tm pt1, newtime;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;
  /*Dia actual*/
  pt1.tm_mday = toInt32(&date[6], 2);
  t1 = mktime(&pt1);

  localtime_s(&newtime, &t1);

  return newtime.tm_wday;
}

int Tool::getDay2(const char* date)
{
  time_t t1;
  struct tm pt1, newtime;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;
  /*Dia actual*/
  pt1.tm_mday = toInt32(&date[6], 2);
  /*pt1.tm_hour = 0;
  pt1.tm_min  = 0;
  pt1.tm_sec  = 0;*/
  t1 = mktime(&pt1);

  localtime_s(&newtime, &t1);

  // sunday..
  if (newtime.tm_wday == 0)
    newtime.tm_wday = 6;
  else
    --newtime.tm_wday;

  return newtime.tm_wday;
}

time_t Tool::timeStamp2Time(const char* timestamp, bool separators)
{
  struct tm tmin;

  if (separators)
  {
    tmin.tm_year = toInt32(&timestamp[1], 4) - 1900;
    tmin.tm_mon = toInt32(&timestamp[6], 2) - 1;
    tmin.tm_mday = toInt32(&timestamp[9], 2);
    tmin.tm_hour = toInt32(&timestamp[12], 2);
    tmin.tm_min = toInt32(&timestamp[15], 2);
    tmin.tm_sec = toInt32(&timestamp[18], 2);
  }
  else
  {
    tmin.tm_year = toInt32(&timestamp[0], 4) - 1900;
    tmin.tm_mon = toInt32(&timestamp[4], 2) - 1;
    tmin.tm_mday = toInt32(&timestamp[6], 2);
    tmin.tm_hour = toInt32(&timestamp[8], 2);
    tmin.tm_min = toInt32(&timestamp[10], 2);
    tmin.tm_sec = toInt32(&timestamp[12], 2);
  }

  return mktime(&tmin);
}

time_t Tool::timeStamp2Time(const char* date, const char* time, bool separators)
{
  struct tm tmin;

  if (separators)
  {
    tmin.tm_year = toInt32(&date[1], 4) - 1900;
    tmin.tm_mon = toInt32(&date[6], 2) - 1;
    tmin.tm_mday = toInt32(&date[9], 2);
    tmin.tm_hour = toInt32(&time[0], 2);
    tmin.tm_min = toInt32(&time[3], 2);
    tmin.tm_sec = toInt32(&time[5], 2);
  }
  else
  {
    tmin.tm_year = toInt32(&date[0], 4) - 1900;
    tmin.tm_mon = toInt32(&date[4], 2) - 1;
    tmin.tm_mday = toInt32(&date[6], 2);
    tmin.tm_hour = toInt32(&time[0], 2);
    tmin.tm_min = toInt32(&time[2], 2);
    tmin.tm_sec = toInt32(&time[4], 2);
  }

  return mktime(&tmin);
}

int Tool::getMonth(char* date)
{
  time_t t1;
  struct tm pt1, newtime;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;
  pt1.tm_mday = toInt32(&date[6], 2);
  pt1.tm_hour = 0;
  pt1.tm_min = 0;
  pt1.tm_sec = 0;
  t1 = mktime(&pt1);

  localtime_s(&newtime, &t1);

  return newtime.tm_mon + 1;
}

int Tool::getLastDay(const char* date)
{
  time_t t1;
  struct tm pt1, newtime;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  /*mes siguiente*/
  pt1.tm_mon = toInt32(&date[4], 2);
  if (pt1.tm_mon == 12)
    pt1.tm_mon = 0;

  pt1.tm_mday = 0;
  pt1.tm_hour = 0;
  pt1.tm_min = 0;
  pt1.tm_sec = 0;
  t1 = mktime(&pt1);

  localtime_s(&newtime, &t1);

  return newtime.tm_mday;
}

int Tool::getMDay(const char* date)
{
  time_t t1;
  struct tm pt1, newtime;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;

  pt1.tm_mday = toInt32(&date[6], 2);
  pt1.tm_hour = 0;
  pt1.tm_min = 0;
  pt1.tm_sec = 0;
  t1 = mktime(&pt1);

  localtime_s(&newtime, &t1);

  return newtime.tm_mday;
}

char* Tool::getCompleteSysTime()
{
  static char completesyshour[20];
  struct timeb timebuffer;
  char timeline[CD_STREAM_256B];

  ftime(&timebuffer);
  ctime_s(timeline, sizeof(timeline), &(timebuffer.time));
  //Wed Feb 13 16:04:43 2002
  sprintf_s(completesyshour, sizeof(completesyshour), "%.8s.%03hu", timeline + 11, timebuffer.millitm);
  return completesyshour;
}

char* Tool::getSysTimemsMT(char* outhour, int32_t ohsize)
{
  struct timeb timebuffer;
  char timeline[CD_STREAM_256B];

  ftime(&timebuffer);
  ctime_s(timeline, sizeof(timeline), &(timebuffer.time));

  sprintf_s(outhour, ohsize, "%.8s%03hu", timeline + 11, timebuffer.millitm);
  removechr(outhour, ':');

  return outhour;
}

char* Tool::getCompleteSysTimeMT(char* szsyshour, int32_t szsyshoursize)
{
  struct timeb timebuffer;
  char timeline[CD_STREAM_256B];

  ftime(&timebuffer);
#if defined(_WIN32) || defined(_WIN64)
  ctime_s(timeline, sizeof(timeline), &(timebuffer.time));
#else
  strcpy_s(timeline, sizeof(timeline), ctime(&(timebuffer.time)));
#endif
  //Wed Feb 13 16:04:43 2002
  sprintf_s(szsyshour, szsyshoursize, "%.8s.%03hu", timeline + 11, timebuffer.millitm);

  return szsyshour;
}

void Tool::completeDateYear(const char* fec_mmdd, char* _fill_year_date_var, int32_t outputsize)
{
  char today[20];
  int ann;
  size_t len;
  char inputdate[CD_STREAM_100B];

  strcpy_s(inputdate, sizeof(inputdate), fec_mmdd);

  if (strlen(fec_mmdd) == 8)
  {
    if (memcmp(fec_mmdd, "0000", 4) == 0)
      strcpy_s(inputdate, sizeof(inputdate), fec_mmdd + 4);
  }

  len = strlen(inputdate);

  getSysDateMT(today, sizeof(today));

  ann = toInt32(today, 4);

  if (strncmp(&today[4], "1231", 4) == 0)
  {
    if (strncmp(inputdate, "01", 2) == 0)
      ann++;
  }
  else if (strncmp(&today[4], "0101", 4) == 0)
  {
    if (strncmp(inputdate, "12", 2) == 0)
      ann--;
  }

  switch (len)
  {
  case 4:
    sprintf_s(_fill_year_date_var, outputsize, "%04d%s", ann, inputdate);
    break;
  case 6:
    sprintf_s(_fill_year_date_var, outputsize, "%02d%s", ann / 100, inputdate);
    break;
  case 8:
    strcpy_s(_fill_year_date_var, outputsize, inputdate);
    break;
  default:
    break;
  }
}

void Tool::date2s(time_t n, char* s, int ssize)
{
  struct tm T, *pT = &T;

#if defined(HAVE_GMTIME_R_NI)
  if (gmtime_r(&n, pT))
    strftime(s, ssize, "%Y-%m-%d", pT);
#elif defined(HAVE_GMTIME_NI)
  if ((pT = gmtime(&n)))
    strftime(s, ssize, "%Y-%m-%d", pT);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timezone tz;
  memset((void*)&tz, 0, sizeof(tz));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%Y-%m-%d", pT);
  }
#else
  if ((pT = localtime(&n)))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%Y-%m-%d", pT);
  }
#endif
#elif defined(HAVE_FTIME)
  struct timeb t;
  memset(&t, 0, sizeof(t));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%Y-%m-%d", pT);
  }
#else
  if(localTime(pT, &n) == 0)
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%Y-%m-%d", pT);
  }
#endif
#elif defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
    strftime(s, ssize, "%Y-%m-%d", pT);
#else
  if ((pT = localtime(&n)))
    strftime(s, ssize, "%Y-%m-%d", pT);
#endif
  else
    strcpy_s(s, ssize, "1969-12-31");
}

void Tool::time2s(time_t n, char* s, int ssize)
{
  struct tm T, *pT = &T;

#if defined(HAVE_GMTIME_R_NI)
  if (gmtime_r(&n, pT))
    strftime(s, ssize, "%H:%M:%SZ", pT);
#elif defined(HAVE_GMTIME_NI)
  if ((pT = gmtime(&n)))
    strftime(s, ssize, "%H:%M:%SZ", pT);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timezone tz;
  memset((void*)&tz, 0, sizeof(tz));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -tz.tz_minuteswest/60+(pT->tm_isdst!=0), (int32_t)fmod(tz.tz_minuteswest, 60));
  }
#else
  if ((pT = localtime(&n)))
  { struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -tz.tz_minuteswest/60+(pT->tm_isdst!=0), (int32_t)fmod(tz.tz_minuteswest, 60));
  }
#endif
#elif defined(HAVE_FTIME)
  struct timeb t;
  memset(&t, 0, sizeof(t));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -t.timezone/60+(pT->tm_isdst!=0), (int32_t)fmod(t.timezone, 60));
  }
#else
  if (localTime(pT, &n) == 0)
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%H:%M:%S", pT);
    sprintf_s((s + strnlen_s(s, ssize)), ssize, "%+03d:%02d", -t.timezone / 60 + (pT->tm_isdst != 0),
            static_cast<int32_t>(fmod(t.timezone, 60)));
  }
#endif
#elif defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
    strftime(s, ssize, "%H:%M:%S", pT);
#else
  if ((pT = localtime(&n)))
    strftime(s, ssize, "%H:%M:%S", pT);
#endif
  else
    strcpy_s(s, ssize, "23:59:59Z");
}

void Tool::dateTime2s(time_t n, char* s, int ssize)
{
  struct tm T, * pT = &T;

#if defined(HAVE_GMTIME_R_NI)
  if (gmtime_r(&n, pT))
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%SZ", pT);
#elif defined(HAVE_GMTIME_NI)
  if ((pT = gmtime(&n)))
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%SZ", pT);
#elif defined(HAVE_GETTIMEOFDAY)
  struct timezone tz;
  memset((void*)&tz, 0, sizeof(tz));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  {
    struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -tz.tz_minuteswest / 60 + (pT->tm_isdst != 0), abs(tz.tz_minuteswest) % 60);
  }
#else
  if ((pT = localtime(&n)))
  {
    struct timeval tv;
    gettimeofday(&tv, &tz);
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -tz.tz_minuteswest / 60 + (pT->tm_isdst != 0), abs(tz.tz_minuteswest) % 60);
  }
#endif
#elif defined(HAVE_FTIME)
  struct timeb t;
  memset(&t, 0, sizeof(t));
#if defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
    sprintf(s + strlen(s), "%+03d:%02d", -t.timezone / 60 + (pT->tm_isdst != 0), (int32_t)fmod(t.timezone, 60));
  }
#else
  if (localTime(pT, &n) == 0)
  {
#ifdef __BORLANDC__
    ::ftime(&t);
#else
    ftime(&t);
#endif
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
    int32_t stl = strnlen_s(s, ssize);
    sprintf_s(s + stl, ssize - stl, "%+03d:%02d", -t.timezone / 60 + (pT->tm_isdst != 0),
      static_cast<int32_t>(fmod(t.timezone, 60)));
  }
#endif
#elif defined(HAVE_LOCALTIME_R)
  if (localtime_r(&n, pT))
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
#else
  if ((pT = localtime(&n)))
    strftime(s, ssize, "%Y-%m-%dT%H:%M:%S", pT);
#endif
  else
    strcpy_s(s, ssize, "1969-12-31T23:59:59Z");
}

void Tool::getSysDateMT(char* buffer, int32_t buffersize)
{
  time_t t;
  char tempdate[CD_STREAM_256B];

  memset(tempdate, 0, sizeof(tempdate));
  
  time(&t);
  dateTime2s(t, tempdate, sizeof(tempdate));
  
  tempdate[10] = 0;
  removechr(tempdate, '-');
  
  strcpy_s(buffer, buffersize, tempdate);
}

void Tool::getSysDateTimeMT(char* datehour, int32_t dhsize)
{
  time_t t;
  char sysdatetime[CD_STREAM_50B];

  time(&t);
  dateTime2s(t, sysdatetime, sizeof(sysdatetime));
  sysdatetime[19] = 0;
  removechr(sysdatetime, '-');
  removechr(sysdatetime, ':');
  removechr(sysdatetime, 'T');

  strcpy_s(datehour, dhsize, sysdatetime);
}

void Tool::getSysTimeMT(char* outhour, int32_t ohsize)
{
  time_t t;
  char systime[CD_STREAM_50B];

  time(&t);
  time2s(t, systime, sizeof(systime));

  systime[8] = 0;
  removechr(systime, ':');

  strcpy_s(outhour, ohsize, systime);
}

char* Tool::getSysTime()
{
  static char syshour[7];
  time_t t;
  struct tm tt;

  time(&t);
  PLATFORM_LOCALTIME(&t, &tt)

  sprintf(syshour, "%02d%02d%02d", tt.tm_hour, tt.tm_min, tt.tm_sec);
  return syshour;
}

int Tool::getNextDate(char* date, int ndays)
{
  time_t t1;
  struct tm pt1, pt2;

  memset(&pt1, 0, sizeof(struct tm));

  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;
  pt1.tm_mday = toInt32(&date[6], 2) + ndays;
  pt1.tm_hour = 0;
  pt1.tm_min = 0;
  pt1.tm_sec = 0;
  t1 = mktime(&pt1);

  PLATFORM_LOCALTIME(&t1, &pt2)

  sprintf(date, "%04d%02d%02d", pt2.tm_year + 1900, pt2.tm_mon + 1, pt2.tm_mday);

  return 0;
}

char* Tool::get_sysmonthday()
{
  static char sysdate[5];
  time_t t;
  struct tm pt;

  memset(sysdate, 0, sizeof(sysdate));

  time(&t);

  PLATFORM_LOCALTIME(&t, &pt)
  sprintf(sysdate, "%02d%02d", pt.tm_mon + 1, pt.tm_mday);
  return sysdate;
}

char* Tool::getSysDateTimemsMT(char* sysdatehourms, int32_t sdhmssize)
{
  struct timeb timebuffer;
  struct tm pt;

  ftime(&timebuffer);
  PLATFORM_LOCALTIME(&timebuffer.time, &pt)

  sprintf_s(sysdatehourms, sdhmssize, "%04d%02d%02d%02d%02d%02d%03hu", 1900 + pt.tm_year, pt.tm_mon + 1, pt.tm_mday, pt.tm_hour,
          pt.tm_min, pt.tm_sec, timebuffer.millitm);

  return sysdatehourms;
}

char* Tool::getCompleteSysDateTimemsMT(char* sysdatehourms, int32_t sdhmssize)
{
  struct timeb timebuffer;
  struct tm pt;

  ftime(&timebuffer);
  PLATFORM_LOCALTIME(&timebuffer.time, &pt)

  sprintf_s(sysdatehourms, sdhmssize, "%04d.%02d.%02d-%02d:%02d:%02d.%03hu", 1900 + pt.tm_year, pt.tm_mon + 1, pt.tm_mday,
          pt.tm_hour, pt.tm_min, pt.tm_sec, timebuffer.millitm);

  return sysdatehourms;
}

char* Tool::getCompleteSysDate(char middle)
{
  static char completesysdate[14];
  time_t t;
  struct tm pt;

  memset(completesysdate, 0, sizeof(completesysdate));
  time(&t);
  PLATFORM_LOCALTIME(&t, &pt)
  sprintf(completesysdate, "%04d%c%02d%c%02d", 1900 + pt.tm_year, middle, pt.tm_mon + 1, middle, pt.tm_mday);
  return completesysdate;
}

char* Tool::getCompleteSysDateMT(char* szsysdate, char middle)
{
  time_t t;
  struct tm pt;
  time(&t);
  PLATFORM_LOCALTIME(&t, &pt)
  sprintf(szsysdate, "%04d%c%02d%c%02d", 1900 + pt.tm_year, middle, pt.tm_mon + 1, middle, pt.tm_mday);
  return szsysdate;
}

void Tool::date2String(time_t date, const char* separator, char* datestring)
{
  struct tm pt;
  PLATFORM_LOCALTIME(&date, &pt)
  sprintf(datestring, "%04d%s%02d%s%02d", 1900 + pt.tm_year, separator, pt.tm_mon + 1, separator, pt.tm_mday);
}

void Tool::time2String(time_t time, const char* separator, char* timestring)
{
  struct tm pt;
  PLATFORM_LOCALTIME(&time, &pt)
  sprintf(timestring, "%02d%s%02d%s%02d", pt.tm_hour, separator, pt.tm_min, separator, pt.tm_sec);
}

time_t Tool::string2Date(const char* date)
{
  time_t t1;
  struct tm pt1;

  memset(&pt1, 0, sizeof(struct tm));
  pt1.tm_year = toInt32(&date[0], 4) - 1900;
  pt1.tm_mon = toInt32(&date[4], 2) - 1;
  pt1.tm_mday = toInt32(&date[6], 2);

  t1 = mktime(&pt1);

  return t1;
}

time_t Tool::string2Time(const char* time)
{
  time_t t1;
  struct tm pt1;

  memset(&pt1, 0, sizeof(struct tm));
  pt1.tm_hour = toInt32(&time[0], 2);
  pt1.tm_min = toInt32(&time[2], 2);
  pt1.tm_sec = toInt32(&time[4], 2);

  t1 = mktime(&pt1);

  return t1;
}

uint64_t Tool::calculateTimeDifference(bool includems, int format, const char* secondtime, const char* firsttime)
{
  time_t t1, t2;
  struct tm pt1, pt2;
  int ms1 = 0, ms2 = 0;
  double diff = 0.0f;
  uint64_t uidiff = 0;

  // Formato AAAAMMDDhhmmssmmm
  memset(&pt1, 0, sizeof(struct tm));
  pt1.tm_year = toInt32(&firsttime[0], 4) - 1900;
  pt1.tm_mon = toInt32(&firsttime[4], 2) - 1;
  pt1.tm_mday = toInt32(&firsttime[6], 2);
  pt1.tm_hour = toInt32(&firsttime[8], 2);
  pt1.tm_min = toInt32(&firsttime[10], 2);
  pt1.tm_sec = toInt32(&firsttime[12], 2);
  t1 = mktime(&pt1);

  memset(&pt2, 0, sizeof(struct tm));
  pt2.tm_year = toInt32(&secondtime[0], 4) - 1900;
  pt2.tm_mon = toInt32(&secondtime[4], 2) - 1;
  pt2.tm_mday = toInt32(&secondtime[6], 2);
  pt2.tm_hour = toInt32(&secondtime[8], 2);
  pt2.tm_min = toInt32(&secondtime[10], 2);
  pt2.tm_sec = toInt32(&secondtime[12], 2);
  t2 = mktime(&pt2);

  diff = difftime(t2, t1);

  if (includems)
  {
    ms1 = toInt32(secondtime + 14, 3);
    ms2 = toInt32(firsttime + 14, 3);
    ms2 = ms1 - ms2;
  }

  switch (format)
  {
  // ms
  case TOOL_DIFFTIME_MS:
    uidiff = diff * 1000 + ms2;
    break;
  // s
  case TOOL_DIFFTIME_S:
    uidiff = diff;
    break;
  // m
  case TOOL_DIFFTIME_M:
    uidiff = diff / 60;
    break;
  }

  return uidiff;
}

uint64_t Tool::calculateTimeDifference(const char* firsttime, time_t reftime)
{
  time_t t1, t2;
  struct tm pt1;
  double tr;

  if (reftime == 0)
    t2 = time(nullptr);
  else
    t2 = reftime;

  memset(&pt1, 0, sizeof(struct tm));
  pt1.tm_year = toInt32(&firsttime[0], 4) - 1900;
  pt1.tm_mon = toInt32(&firsttime[4], 2) - 1;
  pt1.tm_mday = toInt32(&firsttime[6], 2);
  pt1.tm_hour = toInt32(&firsttime[8], 2);
  pt1.tm_min = toInt32(&firsttime[10], 2);
  pt1.tm_sec = toInt32(&firsttime[12], 2);
  t1 = mktime(&pt1);
  tr = difftime(t2, t1);
  return static_cast<uint64_t>(tr);
}

#if defined(_WIN32) || defined (_WIN64)

uint64_t Tool::calculateTimeDifference(SYSTEMTIME systime)
{
  time_t t1, t2;
  struct tm pt1;
  double tr;

  t2 = time(nullptr);

  memset(&pt1, 0, sizeof(struct tm));
  //FILE *fh=fopen("c:\\ert.txt","a+");
  //fprintf(fh,"%d-%d-%d.%d-%d-%d",systime.wYear,systime.wMonth,systime.wDay,systime.wHour,systime.wMinute,systime.wSecond);

  pt1.tm_year = systime.wYear - 1900;
  pt1.tm_mon = systime.wMonth - 1;
  pt1.tm_mday = systime.wDay;
  pt1.tm_hour = systime.wHour;
  pt1.tm_min = systime.wMinute;
  pt1.tm_sec = systime.wSecond;

  t1 = mktime(&pt1);

  tr = difftime(t2, t1);
  //fprintf(fh,"%f",tr);
  //fclose(fh);

  return static_cast<uint64_t>(tr);
}
#endif

#define TOOL_SECONDS_IN_DAY     86400
#define TOOL_SECONDS_IN_HOUR    3600
#define TOOL_SECONDS_IN_MINUTE  60
#define TOOL_SECONDS            1

void Tool::seconds2DHMS(uint64_t totalseconds, int32_t& days, int32_t& hours, int32_t& minutes, int32_t& seconds)
{
  days = totalseconds / TOOL_SECONDS_IN_DAY;
  hours = (totalseconds % TOOL_SECONDS_IN_DAY) / TOOL_SECONDS_IN_HOUR;
  minutes = ((totalseconds % TOOL_SECONDS_IN_DAY) % TOOL_SECONDS_IN_HOUR) / TOOL_SECONDS_IN_MINUTE;
  seconds = (((totalseconds % TOOL_SECONDS_IN_DAY) % TOOL_SECONDS_IN_HOUR) % TOOL_SECONDS_IN_MINUTE) / TOOL_SECONDS;
}

int Tool::between_date(int32_t format, const char* test, const char* start, const char* end)
{
  char sztest[CD_STREAM_100B];

  if (test == nullptr || test[0] == 0)
    return 0;

  switch (format)
  {
  case DATETIME_FORMAT_DOT:
    break;
  case DATETIME_FORMAT_NORMAL:
    completeDateYear(test, sztest, sizeof(sztest));
    if (strcmp(start, sztest) <= 0 && strcmp(sztest, end) <= 0)
      return 1;
    break;
  }

  return 0;
}

int Tool::same_date(int32_t format, const char* test, const char* date)
{
  char sztest[CD_STREAM_100B];

  switch (format)
  {
  case DATETIME_FORMAT_DOT:
    break;
  case DATETIME_FORMAT_NORMAL:
    completeDateYear(test, sztest, sizeof(sztest));
    if (strcmp(date, sztest) == 0)
      return 1;
    break;
  }

  return 0;
}

int Tool::between_time(int32_t format, const char* test, const char* start, const char* end)
{
  if (test == nullptr || test[0] == 0)
    return 0;

  switch (format)
  {
  case DATETIME_FORMAT_DOT:
    break;
  case DATETIME_FORMAT_NORMAL:
    if (strcmp(start, test) <= 0 && strcmp(test, end) <= 0)
      return 1;
    break;
  }

  return 0;
}

bool Tool::isPrintable(uint8_t* buffer, int64_t buffersize)
{
  int64_t count = 0;
  for (uint8_t* rec = buffer; ACE_OS::ace_isprint(*(rec + count)) && count < buffersize; ++count);
  return !(count ^ buffersize);
}

void Tool::replaceNotPrintable(uint8_t* buffer, int64_t buffersize, char newchar)
{
  int64_t count = 0;
  for (uint8_t* rec = buffer; count < buffersize; ++count)
  {
    if (!ACE_OS::ace_isprint(*(rec + count)))
      rec[count] = newchar;
  }
}

void Tool::byteArray2UTF8(unsigned char* input, int64_t inputsize, unsigned char* output, int64_t& outputsize)
{
  int b = 0;
  unsigned char c = 0;
  int index = 0;

  for (unsigned char* a = input; index < inputsize; a++, index++)
  {
    c = *a;
    if (c < 128)
    {
      output[b] = c;
    }
    else // if (c >= 0x000080 && c <= 0x0007FF)
    {
      //Byte represents the start of an encoded character in the range
      //U+0080 to U+07FF
      output[b++] = ((c >> 6) & 15) | 192;
      output[b] = (c & 63) | 128;
    }
    b++;
  }

  outputsize = b;
}

void Tool::UTF82byteArray(unsigned char* input, int64_t inputsize, unsigned char* output, int64_t& outputsize)
{
  int b = 0;
  unsigned char c = 0;
  int index = 0;

  for (unsigned char* a = input; index < inputsize; a++, index++)
  {
    c = *a;
    if (c > 192)
    {
      // Aqui se debe leer dos bytes para formar uno
      output[b] = ((c & 15) << 6) | (input[index + 1] & 63);
      ++index;
      a++;
    }
    else
    {
      output[b] = c;
    }
    b++;
  }

  outputsize = b;
}

int32_t Tool::getUTF8Size(unsigned char* input, int64_t inputsize)
{
  int b = 0;
  unsigned char c = 0;
  int index = 0;

  for (unsigned char* a = input; index < inputsize; a++, index++)
  {
    c = *a;
    if (c >= 128)
      b++;
    b++;
  }

  return b;
}

//#if defined(_WIN32) || defined(_WIN64)

int Tool::mkdir(const char* dir)
{
  char DirName[CD_STREAM_1K];
  auto p = (char*)dir;
  char* q = DirName;

  memset(DirName, 0, sizeof(DirName));
  while (*p)
  {
    if (('\\' == *p) || ('/' == *p))
    {
      if (':' != *(p - 1))
      {
        ACE_OS::mkdir(DirName);
      }
    }
    *q++ = *p++;
    *q = '\0';
  }

  if (*DirName != 0)
    ACE_OS::mkdir(DirName);

  return 0;
}

#ifdef XXXX

int Tool::mkdir(const char *dir)
{
  /*0 OK 1 ERROR*/
  int result=0;

  result=mkdir(dir,S_IRUSR | S_IWUSR | S_IXUSR | S_IFDIR);

  if(result>=0 || errno==EEXIST)
  {
    result=0;
  }
  else
  {
    char *copy, *p;

    result=0;
    /*set_appmsgEx(BAD,"Creacion recursiva de directorio %s", dir);*/
    /*Se intenta crear el directorio*/

    p = copy = strdup(dir);
    do
    {
      p = strchr (p + 1, '/');
      if (p)
        *p = '\0';
      if (access (copy, F_OK) == -1)
      {
        if (mkdir (copy, 0755) == -1)
        {
          /*set_appmsgEx(BAD,"mkdir %s fallo: %s", dir, strerror(errno));*/
          return -1/*EXIT_IO_ERROR*/;
        }
      }
      if(p)
        *p = '/';
    }
    while (p);

    /*set_appmsgEx(BAD,"No se pudo crear directorio %s - error (%d)",dir,result);*/
  }

  return result;
}

#endif

#if defined(_WIN32) || defined (_WIN64)

int pipe(int32_t pipefd[2])
{
  return 0;
}

int getppid()
{
  return 0;
}

int setpgid(int32_t p, int pid)
{
  return 0;
}

int fork()
{
  return 0;
}

int kill(int32_t pid, int killsignal)
{
  return 0;
}

int waitpid(int32_t pid, int* status, int mode)
{
  return 0;
}

#endif

