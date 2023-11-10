package com.sft.core.general;

import ch.qos.logback.classic.Logger;
import freemarker.template.TemplateException;
import freemarker.template.TemplateExceptionHandler;
import java.io.IOException;
import java.io.Writer;

/**
 *
 * @author David Ricardo
 */
public class TemplateEngineExceptionHandler implements TemplateExceptionHandler
{
  static Logger logger = (Logger)org.slf4j.LoggerFactory.getLogger(TemplateEngineExceptionHandler.class);
  
  @Override
  public void handleTemplateException(TemplateException te, freemarker.core.Environment env, Writer writer) throws TemplateException 
  {
    try
    {
      writer.write("${" + te.getBlamedExpressionString() + "}");
    }
    catch (IOException e)
    {
      throw new TemplateException("handleTemplateException", te, env);
    }
  }
}
