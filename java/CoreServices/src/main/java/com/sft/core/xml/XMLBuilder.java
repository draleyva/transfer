package com.sft.core.xml;

import ch.qos.logback.classic.Logger;
import com.google.common.base.Strings;
import com.google.common.io.Closer;
import com.sft.core.exception.CoreException;
import com.sft.core.general.Function;
import java.io.BufferedReader;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.List;

public class XMLBuilder
{
  static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(XMLBuilder.class);
  public static final String CONFIGURATION_COMMENT_00 = "#";
  public static final String CONFIGURATION_COMMENT_01 = "//";
  public static final String CONFIGURATION_FUNCTION = "@";
  
  protected List<XMLBuilderUnit> builder;
  
  protected XMLBuilder()
  {
    builder = new ArrayList<>();
  }
  
  public void configure(String configuration) throws CoreException
  {
    int line = 0;
    Closer closer = Closer.create();
    
    try
    {
      BufferedReader br = closer.register(new BufferedReader(closer.register(new FileReader(configuration))));
      String currentline;
      XMLBuilderUnit bu = null;
      
      while ((currentline = br.readLine()) != null)
      {
        ++line;
        
        // Leer el archivo de configuración
        if(currentline.startsWith(CONFIGURATION_COMMENT_00) || currentline.startsWith(CONFIGURATION_COMMENT_01))
        {
          if(bu != null)
          {
            builder.add(bu);
            bu = null;
          }
          
          continue;
        }
        
        if(currentline.startsWith(CONFIGURATION_FUNCTION))
        {
          if(bu != null)
          {
            builder.add(bu);
            bu = null;
          }
          
          // preparse line
          currentline = currentline.substring(1);
          
          Function function = Function.parseFunction(currentline);
          
          if(Strings.isNullOrEmpty(function.getName()))
          {
            LOGGER.info("Invalid function. Line {}", line);
            continue;
          }
          
          if(function.getName().contains("include"))
          {
            builder.add(executeInclude(function));
          }
          else if(function.getName().contains("iterate"))
          {
            builder.add(executeIterate(function));
          }
        }
        else
        {
          if(bu == null)
          {
            bu = new XMLBuilderUnit();
          }
          
          if(bu.getStringBuilder().length() > 0)
            bu.getStringBuilder().append("\n");
          
          bu.getStringBuilder().append(currentline);
        }
      }
      
      if(bu != null)
      {
        builder.add(bu);
      }
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
    finally
    {
      try
      {
        closer.close();
      }
      catch(Exception e)
      {
        // unable to manage
      }
    }
  }
  
  static final int FUNCTION_INCLUDE_PARAMETER_FILENAME = 0;
  static final int FUNCTION_INCLUDE_PARAMETER_EXPRESSION = 1;
  
  static final int FUNCTION_ITERATE_PARAMETER_FILENAME = 0;
  static final int FUNCTION_ITERATE_PARAMETER_ITERATOR = 1;
  static final int FUNCTION_ITERATE_PARAMETER_EXPRESSION = 2;
  
  protected String resolvePath(String path)
  {
    return path;
  }
  
  XMLBuilderUnit executeInclude(Function function) throws CoreException
  {
    XMLBuilderUnit bu = new XMLBuilderUnit();
    Closer closer = Closer.create();
    
    try
    {
      BufferedReader br = closer.register(new BufferedReader(new FileReader(resolvePath(function.getParameter().get(FUNCTION_INCLUDE_PARAMETER_FILENAME)))));
    
      String currentline;

      // Primer parametro es el nombre del archivo
      while ((currentline = br.readLine()) != null)
      {
        if(bu.getStringBuilder().length()>0)
          bu.getStringBuilder().append("\n");

        bu.getStringBuilder().append(currentline);
      }

      bu.setExpression(Strings.nullToEmpty(function.getParameter().get(FUNCTION_INCLUDE_PARAMETER_EXPRESSION)));
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
    finally
    {
      try
      {
        closer.close();
      }
      catch(Exception e)
      {
        // unable to manage
      }
    }
    
    return bu;
  }
  
  XMLBuilderUnit executeIterate(Function function) throws CoreException
  {
    XMLBuilderUnit bu = new XMLBuilderUnit();
    
    Closer closer = Closer.create();
    
    try
    {
      BufferedReader br = closer.register(new BufferedReader(new FileReader(resolvePath(function.getParameter().get(FUNCTION_ITERATE_PARAMETER_FILENAME)))));
      String currentline;

      // Primer parametro es el nombre del archivo
      while ((currentline = br.readLine()) != null)
      {
        if(bu.getStringBuilder().length()>0)
          bu.getStringBuilder().append("\n");

        bu.getStringBuilder().append(currentline);
      }

      bu.setIterator(function.getParameter().get(FUNCTION_ITERATE_PARAMETER_ITERATOR));
      bu.setExpression(Strings.nullToEmpty(function.getParameter().get(FUNCTION_ITERATE_PARAMETER_EXPRESSION)));
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
    finally
    {
      try
      {
        closer.close();
      }
      catch(Exception e)
      {
        // unable to manage
      }
    }
    return bu;
  }
}
