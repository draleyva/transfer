#pragma once

class BaseCoreServices
{
protected:
  BaseCoreServices();
public:
#if defined(_WIN32) || defined(_WIN64)
  static void ansi2Bstr(const char* sz, BSTR* pbstr);
  static void bstr2Ansi(BSTR* pbstr, char* sz, IN size_t cch);
  static HRESULT variant2ByteArray(VARIANT request, BYTE* pData, DWORD* pdataLength);
  static HRESULT byteArray2Variant(const BYTE* pData, DWORD dataLength, VARIANT* ppResults);
#endif
  static int32_t openFile(FILE** fh, const char* filename, const char* mode);
  static int32_t getenv(const char *vn, char *v, int32_t vsize);
  static void trim(char* data, char char2remove = ' ');
  static void trimlr(char* field, int32_t fieldsize, char chartoremove = ' ');
  static void triml(char* field, int32_t fieldsize, char t);
  static void trace(FILE* tf, const char* format, ...);
  static int32_t localTime(struct tm* const tmDest, time_t const* const sourceTime);
  static int32_t getfirstpath(const char* filename, char *path, int32_t pathsize);
  static bool isFile(const char* s);
  static int replacechr(char* str, int f, int n);
  static int replacechr(char* str, int f, int n, int& occ);
  static bool isNullOrEmpty(const char* ptr);
  static void wchar2char(std::wstring wstr, char* output, int& outputsize);
  static void char2wchar(const char* in, std::wstring& output);
};
