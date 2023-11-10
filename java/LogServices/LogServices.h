#ifndef LOGSERVICES_H
#define LOGSERVICES_H

#if defined(_WIN32) || defined(_WIN64)

#ifdef LOGSERVICES_EXPORTS
#define LOGSERVICES_API __declspec(dllexport)
#else
#define LOGSERVICES_API __declspec(dllimport)
#endif
#else
#define LOGSERVICES_API
#endif

#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/ndc.h>
#include <log4cplus/mdc.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/thread/threads.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/initializer.h>

//using namespace std;
//using namespace log4cplus;
//using namespace log4cplus::helpers;

class LOGSERVICES_API LogServices
{
public:
  LogServices(void);
  static log4cplus::SharedFileAppenderPtr appender;
  void static configure(const char* configfile);
  void static configure(const char* product, const char* installation, const char* app);
  void static shutdown();
};

#define LOGSERVICES_DECLARE_LOGGER(L, X) log4cplus::Logger L = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(X));
#define LOGSERVICES_DECLARE_LOGGER_REFERENCE(L) extern log4cplus::Logger L;


#define LS_ERROR(y) LOG4CPLUS_ERROR(LOGGER, y)
#define LS_TRACE(y) LOG4CPLUS_TRACE(LOGGER, y)
#define LS_INFO(y) LOG4CPLUS_INFO(LOGGER, y)
#define LS_WARN(y) LOG4CPLUS_WARN(LOGGER, y)
#define LS_DEBUG(y) LOG4CPLUS_DEBUG(LOGGER, y)
#define LS_FATAL(y) LOG4CPLUS_FATAL(LOGGER, y)

#define LSEXT_ERROR(x, y) LOG4CPLUS_ERROR(x, y)
#define LSEXT_TRACE(x, y) LOG4CPLUS_TRACE(x, y)
#define LSEXT_INFO(x, y) LOG4CPLUS_INFO(x, y)
#define LSEXT_WARN(x, y) LOG4CPLUS_WARN(x, y)
#define LSEXT_DEBUG(x, y) LOG4CPLUS_DEBUG(x, y)
#define LSEXT_FATAL(x, y) LOG4CPLUS_FATAL(x, y)

#define LOGSERVICES_INITIALIZER() log4cplus::Initializer initializer;

#endif
