package com.sft.core.general;

import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;
import com.sft.log.LogUtil;
import freemarker.core.InvalidReferenceException;
import freemarker.ext.beans.BeansWrapper;
import freemarker.ext.beans.BeansWrapperBuilder;
import freemarker.template.Configuration;
import freemarker.template.Template;
import freemarker.template.TemplateExceptionHandler;
import java.io.StringReader;
import java.io.StringWriter;
import java.io.Writer;
import java.nio.charset.Charset;

/**
 *
 * @author David Ricardo
 * Dorg.freemarker.loggerLibrary=slf4j. See SYSTEM_PROPERTY_NAME_LOGGER_LIBRARY for more.
 */
public class TemplateEngine
{
  static Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(TemplateEngine.class);
  
  static final TemplateExceptionHandler TEMPLATE_EXCEPTION;
  static final Configuration TEMPLATE_DEFAULT_CONFIGURATION;
  static final Configuration TEMPLATE_CUSTOM_CONFIGURATION;
  
  static
  {  
    System.setProperty("org.freemarker.loggerLibrary", "slf4j");
    
    LogUtil.setLogLevel(new String[]{"freemarker.beans", "freemarker.runtime"}, Level.OFF);
    
    BeansWrapperBuilder builder = new BeansWrapperBuilder(Configuration.VERSION_2_3_30);
    builder.setUseModelCache(true);
    builder.setExposeFields(true);
    BeansWrapper beansWrapper = builder.build();
    
    TEMPLATE_DEFAULT_CONFIGURATION = new Configuration(Configuration.VERSION_2_3_30);
    TEMPLATE_DEFAULT_CONFIGURATION.setTemplateUpdateDelayMilliseconds(Integer.MAX_VALUE);
    TEMPLATE_DEFAULT_CONFIGURATION.setObjectWrapper(beansWrapper);
    TEMPLATE_DEFAULT_CONFIGURATION.setClassicCompatible(false);
    
    TEMPLATE_CUSTOM_CONFIGURATION = new Configuration(Configuration.VERSION_2_3_30);
    TEMPLATE_EXCEPTION = new TemplateEngineExceptionHandler();
    TEMPLATE_CUSTOM_CONFIGURATION.setTemplateExceptionHandler(TEMPLATE_EXCEPTION);
    TEMPLATE_CUSTOM_CONFIGURATION.setTemplateUpdateDelayMilliseconds(Integer.MAX_VALUE);
    TEMPLATE_CUSTOM_CONFIGURATION.setObjectWrapper(beansWrapper);
    TEMPLATE_CUSTOM_CONFIGURATION.setClassicCompatible(false);
  }
  
  protected TemplateEngine()
  {
  }
  
  public static String expandVariable(Object entry, String variable)
  {
    return expandVariable(entry, Charset.defaultCharset().name(), variable, true);
  }
  
  public static String expandVariable(Object entry, String variable, boolean custom)
  {
    return expandVariable(entry, Charset.defaultCharset().name(), variable, custom);
  }
  
  public static String expandVariable(Object entry, String encoding, String variable)
  {
    return expandVariable(entry, encoding, variable, true);
  }
  
  public static String expandVariable(Object entry, String encoding, String variable, boolean custom)
  {
    Writer out = new StringWriter();
      
    try
    {
      Template template = new Template("expandVariable", new StringReader(variable), custom?TEMPLATE_CUSTOM_CONFIGURATION:TEMPLATE_DEFAULT_CONFIGURATION);
      template.process(entry, out);
    }
    catch(InvalidReferenceException e)
    {
      // Esto está manejado en lo posible
    }
    catch(Exception e)
    {
      LOGGER.warn("expandVariable->", e);
    }
    
    return out.toString();
  }
}
