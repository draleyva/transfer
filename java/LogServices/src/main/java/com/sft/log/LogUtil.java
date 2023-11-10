package com.sft.log;

import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author David Ricardo
 */
public class LogUtil
{
  protected LogUtil()
  {  
  }
  
  public static void disableLog(String [] classes)
  {
    setLogLevel(classes, Level.OFF);
  }
  
  public static void setLogLevel(String [] classes, Level level)
  {
    if(level == null)
      return;
    
    Logger root;
    for(String c : classes)
    {
      root = (Logger)LoggerFactory.getLogger(c);
      root.setLevel(level);
    }
  }
  
  public static void setLogLevel(Class [] classes, Level level)
  {
    if(level == null)
      return;
    
    Logger root;
    for(Class c : classes)
    {
      root = (Logger)LoggerFactory.getLogger(c);
      root.setLevel(level);
    }
  }
}
