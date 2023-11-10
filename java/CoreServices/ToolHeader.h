/*CONTROLHEADER*/

#ifndef TOOLHEADER_H
#define TOOLHEADER_H

#if !defined(_WIN32) && !defined(_WIN64)
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>

#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <math.h>
#include <ctype.h>

#include <ace/OS_NS_time.h>
#include <ace/OS_NS_signal.h>
#include <ace/OS_NS_string.h>
#include <ace/OS_NS_sys_stat.h>
#include <ace/OS_NS_stdlib.h>
#include <ace/OS_NS_ctype.h>
#include <ace/OS_NS_stdio.h>
#include <ace/ACE.h>
#include <stdint.h>
#include "ToolPlatform.h"

#if defined(HAVE_FTIME)
#include <sys/timeb.h>
#endif

#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)

#include <wtypes.h>
#include <basetsd.h>
#include <WinBase.h>
#include <windef.h>
#include <share.h>

#ifndef MINGW

#include <intrin.h>
#include <atlcomcli.h>
#include <comutil.h>
#include <strsafe.h>
#include <ShlObj.h>

#if defined(_WIN64)
#pragma pack(push,8)
#endif
#include <Psapi.h>
#if defined(_WIN64)
#pragma pack(pop)
#endif
#include <Pdh.h>
#endif

#else

// Linux/AIX

#include <unistd.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <pthread.h>

#ifndef AIX
#include <sys/syscall.h>
#endif

#endif

#if defined(_WIN32) || defined(_WIN64)

#include <io.h>
#include <tchar.h>

#else

//typedef unsigned long DWORD;
typedef long HRESULT;
typedef int BOOL;
typedef int INT;
typedef const char * LPCSTR;

typedef size_t SIZE_T;
typedef unsigned long ULONG;
//typedef unsigned long DWORD;
typedef struct linger LINGER;
typedef unsigned int UINT;

#define TRUE 1
#define FALSE 0

#ifndef __GNUC__
typedef int bool;
#endif

#endif //WIN32

#if defined(_WIN32) || defined(_WIN64)
#define CW2T_NULL(x) (x?CW2T(x):"NULL")
#else
#endif

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifdef true
#undef true
#define true 1
#else
#define true 1
#endif

#ifdef false
#undef false
#define false 0
#else
#define false 0
#endif

#ifndef S_OK
#define S_OK 0
#define S_FALSE 1
#endif

#define S_FAIL -1

#ifndef STR_S
#define STR_S(X) X,sizeof(X)
#endif

#define absmajor(x,y) (abs(x)>abs(y))?y:x
#define absisminor(x,y) abs(x) <= abs(y)

#define CHECK_BIT(var__,pos__) ((var__) & (1<<(pos__)))
#define SECONDS2MSECONDS(x) (x*1000)
#define MSECONDS2SECONDS(x) (x/1000)
#define ISTIMEOUTSMS(x, y) ((uint64_t)(1000*x) < (uint64_t)y)
#define MINROUNDVAL 1e-5
#define ROUND(x) (abs(x) <= 0.000000001f?x:(x>0?x+MINROUNDVAL:x-MINROUNDVAL))
#define DELTAZERO 0.00000001f
#define NOTEQUAL2ZERO(x) (abs(x) > DELTAZERO)
#define INPUTFUNC 0x23100000
#define OUTPUTFUNC   0x23200000
#define TRUEFALSESTRING(x) (x?"true":"false")
#define PREDEFTOKEN "!*>"
#define GLOBALSEPARATOR_0 ';'
#define GLOBALSEPARATOR_0_S ";"
#define GLOBALSEPARATOR_1 '-'
#define GLOBALSEPARATOR_1_S "-"
#define GLOBALDATEYYYYMMDDLENGTH 8
#define GLOBALDATEHHMMSSLENGTH 6
#define GLOBALNONE "NONE"

#define GLOBALTRUE "1"

#endif

#ifndef _WIN32_WINNT            // Especifica que la plataforma mínima requerida es Windows Vista.
#define _WIN32_WINNT 0x0501     // Cambiar al valor apropiado correspondiente a otras versiones de Windows.
#endif
