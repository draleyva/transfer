package com.sft.log;

import ch.qos.logback.classic.LoggerContext;
import ch.qos.logback.classic.util.ContextInitializer;
import ch.qos.logback.core.util.StatusListenerConfigHelper;
import java.io.File;
import java.net.URL;

public class LogConfigurator 
{
  //static Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(LogConfigurator.class);
  
  protected LogConfigurator()
  {
  }
  
  public static void configure(String configfile) throws Exception
  {
    File f = new File(configfile);
    URL resourcefile = f.toURI().toURL();
    
    LoggerContext lc = (LoggerContext) org.slf4j.LoggerFactory.getILoggerFactory();
    lc.reset();
    StatusListenerConfigHelper.installIfAsked(lc);
    
    ContextInitializer configurator = new ContextInitializer(lc);
    
    configurator.configureByResource(resourcefile);
    lc.start();
    
    // DO NOT REMOVE
    /*
    for(Status s : lc.getStatusManager().getCopyOfStatusList())
    {
      System.out.println("msg : "+s.getMessage());
    }
    */
  }
}
