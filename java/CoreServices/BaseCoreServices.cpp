#include "CoreDefinition.h"
#include "ToolHeader.h"
#include <memory>
#include "GenericArray.h"
#include "BaseCoreServices.h"

using namespace std;

BaseCoreServices::BaseCoreServices()
{
}

#if defined(_WIN32) || defined(_WIN64)

//---------------------------------------------------------
// Convert an ANSI string to a BSTR
//---------------------------------------------------------
void BaseCoreServices::ansi2Bstr(
  IN const char* sz,
  OUT BSTR* pbstr
)
{
  // Build a CComBSTR object assigning the input
  // ANSI string (the constructor converts from
  // ANSI to BSTR)
  CComBSTR bstr(sz);


  // Copy to destination BSTR parameter,
  // releasing the BSTR
  *pbstr = bstr.Detach();
}

//---------------------------------------------------------
// Convert a BSTR to ANSI.
// The caller must provide memory buffer for ANSI,
// and the buffer size (in char's)
//---------------------------------------------------------
void BaseCoreServices::bstr2Ansi(
  IN BSTR* pbstr,
  OUT char* sz,
  IN size_t cch
)
{
  // Build an helper CComBSTR object with the input string
  CComBSTR bstr(*pbstr);

  // Convert from Unicode to ANSI
  CW2A ansi(bstr);

  // Copy ANSI to destination buffer
  StringCchCopy(sz, cch, ansi);
}

HRESULT BaseCoreServices::variant2ByteArray(VARIANT request, BYTE* pData, DWORD* pdataLength)
{
  HRESULT hr = S_OK;
  PVOID pvData = nullptr;
  long upperBounds = 0;
  long lowerBounds = 0;
  size_t length = 0;

  USES_CONVERSION;

  switch (request.vt)
  {
  case VT_ARRAY | VT_UI1:
    if (SafeArrayGetDim(request.parray) != 1 || request.parray == nullptr)
    {
      // Arreglo de mas de una dimensión
      return E_POINTER;
    }
    SafeArrayGetLBound(request.parray, 1, &lowerBounds);
    SafeArrayGetUBound(request.parray, 1, &upperBounds);
    length = upperBounds - lowerBounds + 1;
    hr = SafeArrayAccessData(request.parray, &pvData);
    if (hr != S_OK)
    {
      return hr;
    }

    ::CopyMemory(pData, pvData, length);
    *pdataLength = static_cast<DWORD>(length);
    hr = SafeArrayUnaccessData(request.parray);
    break;
  case VT_BSTR:

    pvData = (unsigned char*)W2A(request.bstrVal);
    if (pvData)
      length = strlen(static_cast<const char*>(pvData));
    ::CopyMemory(pData, pvData, length);
    *pdataLength = static_cast<DWORD>(length);
    break;
  default:
    hr = E_INVALIDARG;
    break;
  }

  return hr;
}

HRESULT BaseCoreServices::byteArray2Variant(const BYTE* pData, DWORD dataLength, VARIANT* ppResults)
{
  SAFEARRAYBOUND sab;
  PVOID pvData = nullptr;
  HRESULT hr = S_OK;
  SAFEARRAY* pSa = nullptr;

  sab.cElements = dataLength;
  sab.lLbound = 0;

  pSa = SafeArrayCreateEx(VT_UI1, 1, &sab, nullptr);
  if (pSa == nullptr)
    return E_OUTOFMEMORY;

  hr = SafeArrayAccessData(pSa, &pvData);

  if (SUCCEEDED(hr))
  {
    ::CopyMemory(pvData, pData, dataLength);

    hr = SafeArrayUnaccessData(pSa);
  }
  else
    SafeArrayDestroy(pSa);

  if (SUCCEEDED(hr))
  {
    VariantInit(ppResults);

    (*ppResults).parray = pSa;
    (*ppResults).vt = VT_ARRAY | VT_UI1;
  }

  return hr;
}

#endif

int32_t BaseCoreServices::localTime(struct tm* const tmDest, time_t const* const sourceTime)
{
#if defined(_WIN32) || defined(_WIN64)
  return localtime_s(tmDest, sourceTime);
#else
  struct tm* local = localtime(sourceTime);
  memcpy(tmDest, local, sizeof(struct tm));
  return local != nullptr ? 0 : -1;
#endif
}

int32_t BaseCoreServices::getenv(const char* vn, char* v, int32_t vsize)
{
#if defined(_WIN32) || defined(_WIN64)
  size_t rv = 0;
  int32_t result = ::getenv_s(&rv, v, vsize, vn);
  if (result != 0)
    return result;
#else
  char* ptr;
  ptr = ::getenv(vn);
  if (ptr == NULL)
    return -1;
  strcpy_s(v, vsize, ptr);
#endif
  return 0;
}

void BaseCoreServices::wchar2char(std::wstring wstr, char* output, int& outputsize)
{
  std::string strValue;
  strValue.assign(wstr.begin(), wstr.end());

  outputsize = snprintf(output, outputsize, "%s", strValue.c_str());
}

void BaseCoreServices::char2wchar(const char* in, std::wstring& output)
{
  std::string strValue = in;
  std::wstring wsTmp(strValue.begin(), strValue.end());

  output = wsTmp;
}

void BaseCoreServices::trim(char* data, char char2remove)
{
  if (data == nullptr)
    return;

  int i;

  for (i = static_cast<int>(strlen(data)) - 1; i >= 0 && data[i] == char2remove; i--)
    data[i] = 0;
}

void BaseCoreServices::trimlr(char* data, int32_t datasize, char chartoremove)
{
  int32_t i = 0;
  int32_t length = 0;

  if (data == nullptr || (length = strlen(data)) == 0)
    return;

  trim(data, chartoremove);

  while (data[i++] == chartoremove);

  if (i > 0)
  {
    length += 1;
    char* temp = nullptr;
    temp = new char[length];
    memset(temp, 0, length);
    memcpy(temp, (data + i - 1), strnlen_s(data + i - 1, datasize));
    strcpy_s(data, datasize, temp);
    delete[] temp;
  }
}

void BaseCoreServices::triml(char* data, int32_t datasize, char t)
{
  int32_t i = 0;
  int32_t length = 0;

  if (data == nullptr || (length = strlen(data)) == 0)
    return;

  while (data[i++] == t);

  if (i > 0)
  {
    length += 1;
    char* temp = nullptr;
    temp = new char[length];
    memset(temp, 0, length);
    memcpy(temp, (data + i - 1), strlen(data + i - 1));
    strcpy_s(data, datasize, temp);
    delete[] temp;
  }
}

void BaseCoreServices::trace(FILE* tf, const char* format, ...)
{
  va_list arglist;
  char szdebugger[CD_STREAM_4K];

  va_start(arglist, format);
  vsnprintf(szdebugger, sizeof(szdebugger), format, arglist);
  va_end(arglist);

  if (tf == nullptr)
    tf = stdout;

  fprintf(tf, "%s\n", szdebugger);
  fflush(tf);
}

int32_t __gfp(const char* filename, const char *env, const char* separator, char *path, int32_t pathsize)
{
  int32_t result = BaseCoreServices::getenv(env, path, pathsize);

  if (result != 0)
    return result;

  char firstpath[CD_STREAM_2K];
  char* tok = nullptr;
  char* context = nullptr;

  tok = strtok_s(path, separator, &context);

  while (tok != nullptr)
  {
    if (tok[strlen(tok) - 1] == '/' || tok[strlen(tok) - 1] == '\\')
      sprintf_s(firstpath, sizeof(firstpath), "%s%s", tok, filename);
    else
      sprintf_s(firstpath, sizeof(firstpath), "%s/%s", tok, filename);

    BaseCoreServices::replacechr(firstpath, '\\', '/');

    FILE* fh = nullptr;
    
    if (BaseCoreServices::openFile(&fh, firstpath, "r") == 0)
    {
      fclose(fh);
      strcpy_s(path, pathsize, firstpath);
      return 0;
    }

    tok = strtok_s(nullptr, separator, &context);   
  }

  return -1;
}

int32_t BaseCoreServices::openFile(FILE** fh, const char* filename, const char* mode)
{
#if defined(_WIN32) || defined(_WIN64)
  return fopen_s(fh, filename, mode);
#else
  * fh = fopen(filename, mode);
  return *fh != NULL ? 0 : -1;
#endif
}

// dleyva CHECK REVISAR ESTA FUNCION
int32_t BaseCoreServices::getfirstpath(const char* filename, char* path, int32_t pathsize)
{
  GENERICARRAY(char, librarypath, CD_STREAM_16K);
  char separator[CD_STREAM_100B];
  int32_t result;

#if defined(_WIN32) || defined(_WIN64)
  strcpy_s(separator, sizeof(separator), ";");
#else
  strcpy_s(separator, sizeof(separator), ":");
#endif

  result = __gfp(filename, "LIBPATH", separator, librarypath->array, librarypath->limit());
  if (result == 0)
  {
    strcpy_s(path, pathsize, librarypath->array);
    return 0;
  }

  result = __gfp(filename, "LD_LIBRARY_PATH", separator, librarypath->array, librarypath->limit());
  if (result == 0)
  {
    strcpy_s(path, pathsize, librarypath->array);
    return 0;
  }

  result = __gfp(filename, "PATH", separator, librarypath->array, librarypath->limit());
  if (result == 0)
  {
    strcpy_s(path, pathsize, librarypath->array);
    return 0;
  }
  
  strcpy_s(path, pathsize, filename);

  return -1;
}

bool BaseCoreServices::isFile(const char* s)
{
  if (s[0] == 0)
    return false;

  FILE* fh = nullptr;

  if (openFile(&fh, s, "r") != 0)
    return false;

  fclose(fh);

  return true;
}

int BaseCoreServices::replacechr(char* str, int f, int n)
{
  char* p = str;
  while (*p)
  {
    if (*p == f)
      *p = n;
    ++p;
  }
  return 0;
}

int BaseCoreServices::replacechr(char* str, int f, int n, int& occ)
{
  int localocc = 0;
  char* p = str;
  while (*p)
  {
    if (*p == f)
    {
      *p = n;
      ++localocc;
    }
    ++p;
  }

  occ = localocc;
  return 0;
}

bool BaseCoreServices::isNullOrEmpty(const char* ptr)
{
  return ptr == nullptr || ptr[0] == 0;
}
