#pragma once

#if defined(_WIN32) || defined(_WIN64)
#ifdef MINGW
#define WIN32_PORTABLE
#endif
#else
#define WIN32_PORTABLE
#define GetLastError() errno
#endif

#ifdef WIN32_PORTABLE
#define strnlen_s(x, y) strnlen(x, y)
#define strtok_s(x, y, z) strtok_r(x, y, z)
#define strcpy_s(x, y, z) strcpy(x, z)
#define strcat_s(x, y, z) strcat(x, z)
#define strncpy_s(x, y, z, n) strncpy(x, z, n)
#define sprintf_s(buffer, buffer_size, stringbuffer, ...) (sprintf(buffer, stringbuffer, ## __VA_ARGS__))
#define vsprintf_s(buffer, buffer_size, stringbuffer, _valist) (vsprintf(buffer, stringbuffer, _valist))
#define localtime_s(x, y) {struct tm* temptm__; temptm__=localtime(y); memcpy(x, temptm__, sizeof(struct tm));}
#define getenv_s(x, y, z, w) {char *temp__;temp__=getenv(w);if(temp__!=NULL)strcpy(y, temp__);*x=0;}
#define ctime_s(x, y, z) strcpy_s(x, y, ctime(z))
#endif

#define memset2(x) ::memset(x,0,sizeof(x))
#define memset3(x) ::memset(&x,0,sizeof(x))
#define BUFFCPY(x, y) strcpy_s((char*)x, sizeof(x), y)
#define STRCPY(x, y) strcpy_s(x, sizeof(x), y)

#if !defined(_WIN32) && !defined(_WIN64)
#ifndef INFINITE
#define INFINITE 0xFFFFFFFF
#endif
#endif

// HAVE_GETTIMEOFDAY

#if defined(TRU64)
#define HAVE_GETTIMEOFDAY 1
#endif

// HAVE_FTIME

#if defined(_WIN32) || defined(_WIN64) || defined(AIX) || defined (LINUX)
#if !defined HAVE_FTIME
#define HAVE_FTIME 1
#endif
#endif

// HAVE_MS_THREAD

#if defined(_WIN32) || defined(_WIN64)
#if !defined HAVE_MS_THREAD
#define HAVE_MS_THREAD 1
#endif
#endif

#if !defined HAVE_THREAD
#define HAVE_THREAD 1
#endif

#if defined(_WIN32) || defined(_WIN64)
#define HAVE_GMTIME
#else

#define HAVE_GMTIME_R
#define HAVE_LOCALTIME_R

#endif

#define PLATFORM_LOCALTIME(x, y) ACE_OS::localtime_r(x, y);
