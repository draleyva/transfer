#include "stdafx.h"
#include <CoreServices/CoreDefinition.h>
#include <CoreServices/ToolHeader.h>
#include <CoreServices/Tool.h>
#include <ConfigurationRoot/ConfigurationRoot.h>
#include "LogServices.h"

#if defined(_WIN32) || defined(_WIN64)
#pragma comment (lib, "ACE")
#pragma comment (lib, "CoreServices")
#pragma comment (lib, "log4cplus")
#pragma comment (lib, "ConfigurationRoot")
#endif

log4cplus::SharedFileAppenderPtr LogServices::appender;

LogServices::LogServices()
{
}

void LogServices::configure(const char* configfile)
{
}

// TODO : read the groovy file
void LogServices::configure(const char* product, const char* installation, const char* app)
{
  char logpath[CD_STREAM_2K];
  char logfile[CD_STREAM_2K];

  ConfigurationRoot::getLogPath(product, installation, logpath, sizeof(logpath));

  sprintf(logfile, "%s/%s/%s", logpath, app, app);

  //SharedObjectPtr<Appender> rollingappender(new RollingFileAppender(LOG4CPLUS_TEXT(logfile), MAXLONG, 5, true, true));
  //appender = new RollingFileAppender(LOG4CPLUS_TEXT(logfile), MAXLONG, 5, true, true);
  log4cplus::tstring datepattern = LOG4CPLUS_TEXT("%Y-%m-%d.log");

  appender = new log4cplus::DailyRollingFileAppender(
    LOG4CPLUS_TEXT(logfile), log4cplus::DailyRollingFileSchedule::DAILY, true, 1, true, true, datepattern);

  //log4cplus::tstring pattern = LOG4CPLUS_TEXT("%d{[%Y.%m.%d-%H:%M:%S.%Q]} %-6p [%t] %c{2} %%%x%% - %X{key} - %m [%l]%n");
  //pattern = "[%d{yyyy.MM.dd-HH:mm:ss.SSS}] %-6level %-18.18thread %msg%n"
  log4cplus::tstring pattern = LOG4CPLUS_TEXT("%D{[%Y.%m.%d-%H:%M:%S.%q]} %-6p %-18t %m%n");

  appender->setLayout(std::make_unique<log4cplus::PatternLayout>(pattern));

  log4cplus::Logger::getRoot().addAppender(log4cplus::SharedAppenderPtr(appender.get()));

  // check this
  log4cplus::Logger::getRoot().setLogLevel(log4cplus::ALL_LOG_LEVEL);
}

void LogServices::shutdown()
{
}
