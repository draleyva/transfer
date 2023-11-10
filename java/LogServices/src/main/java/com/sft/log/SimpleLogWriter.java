package com.sft.log;

import ch.qos.logback.classic.Logger;
import com.google.common.base.Strings;
import java.io.IOException;
import java.io.Writer;

/**
 *
 * @author David Ricardo
 */
public class SimpleLogWriter extends Writer
{
  static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(SimpleLogWriter.class);
  
  @Override
  public void write(char[] cbuf, int off, int len) throws IOException 
  {
    String text = new String(cbuf, off, len);
    
    if(!Strings.isNullOrEmpty(text))
    {
      LOGGER.info("{}", text);
    }
  }

  @Override
  public void flush() throws IOException 
  {
    // default implementation
  }

  @Override
  public void close() throws IOException 
  {
    // default implementation
  }
}
