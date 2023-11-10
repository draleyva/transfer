package com.sft.core.general;

import ch.qos.logback.classic.Logger;
import com.google.common.base.CharMatcher;
import com.google.common.base.Splitter;
import com.google.common.base.Strings;
import com.sft.core.configuration.ConfigurationProperties;
import com.sft.core.converter.ByteConvert;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;

public class SystemVariable 
{
  public static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(SystemVariable.class);
  static final String VARIABLE = "Variable";
  static final String COMMENT = "#";
  Splitter variableSplitter = Splitter.onPattern(" ?= ?");
  
  Map<String, String> variableMap = new LinkedHashMap<>();
  
	public void load(String varfile)
	{
		load(varfile, Charset.defaultCharset().name());
	}
	
  public void load(String varfile, String encoding)
  {
    try(BufferedReader br = new BufferedReader(new InputStreamReader(new DataInputStream(new FileInputStream(varfile)), encoding)))
    {
      String line;
      boolean checkingvariable = false;
      Iterator<String> itor;
      String variable;
      String value;
      
      while ((line = br.readLine()) != null)
      {
        if(!checkingvariable && line.startsWith(COMMENT) && line.toLowerCase().contains(VARIABLE.toLowerCase()))
        {
          checkingvariable = true;
        }
        
        if(!checkingvariable)
          continue;
        
        // Procesar
        itor = variableSplitter.split(line).iterator();

        if(!itor.hasNext())
          continue;
        
        variable = itor.next();
        
        if(!itor.hasNext() || variable.startsWith(COMMENT))
          continue;
        
        // Resolver valor de acuerdo a la definición
        
        value = expandVariable(itor.next());
        
        variableMap.put(variable, value);
      }
    }
    catch (IOException e)
    {
      // unable to manage
    }
  }
  
  public String getValue(String variable)
  {
    return Strings.nullToEmpty(variableMap.get(variable));
  }
  
  public void setValue(String variable, String value)
  {
    variableMap.put(variable, value);
  }

  static Splitter splitter = Splitter.onPattern(" ?\\{ ?").trimResults(CharMatcher.anyOf("}"));
  static Splitter parameterSplitter = Splitter.on("|");
  
  public static final String PROPERTY = "PROPERTY";
  public static final String SYSTEMPROPERTY = "SYSTEMPROPERTY";
  public static final String READDATA = "READDATA";
  public static final int BUFFER_SIZE = 4096;
  
  public static String expandVariable(String value)
  {
    Iterator<String> itor = splitter.split(value).iterator();
    String function = null;
    String parameters = null;
    Map<Integer, String> map = new HashMap<>();
    
    while(itor.hasNext())
    {
      if(function == null)
      {
        function = itor.next();
      }
      else if(parameters == null)
      {
        parameters = itor.next();
        Iterator<String> paramitor = parameterSplitter.split(parameters).iterator();
        int paramindex = 0;
        
        while(paramitor.hasNext())
        {
          map.put(paramindex++, paramitor.next());
        }
      }
      else
        break;
    }
    
    if(function != null)
    {
      String savedvalue = value;
      
      value = null;
      
      // El valor se debe resolver con una función
      // En el caso que no se resuelva se devolverá null
      if(function.equalsIgnoreCase(SYSTEMPROPERTY))
      {
        return SystemProperty.get(parameters);
      }
      else if(function.equalsIgnoreCase(PROPERTY))
      {
        ConfigurationProperties prop = new ConfigurationProperties();
        
        try
        {
          prop.load(map.get(0));
          
          return prop.getString(map.get(1));
        }
        catch(Exception e)
        {
          LOGGER.error("expandVariable ->", e);
        }
      }
      else if(function.equalsIgnoreCase(READDATA))
      {
        try(DataInputStream in = new DataInputStream(new FileInputStream(map.get(0))))
        {
          byte [] buffer = new byte [BUFFER_SIZE];
          int buffersize;
          
          in.skipBytes(Integer.parseInt(map.get(1)));
          buffersize = in.read(buffer, 0, Integer.parseInt(map.get(2)));
          
          if(buffersize > 0)
          {
            return ByteConvert.bytes2String(buffer, buffersize, "UTF-8");
          }
        }
        catch(Exception e)
        {
          LOGGER.error("expandVariable ->", e);
        }
      }
      else
      {
        value = savedvalue;
      }
    }
    
    return value;
  }
  
  public boolean canExpand(String line)
  {
    for(String key : variableMap.keySet())
    {
      if(line.contains(key))
        return true;
    }
    
    return false;
  }
  
  public String expand(String line)
  {
    if(Strings.isNullOrEmpty(line))
      return line;
    
    String local = line;
    
    for (Entry<String, String> entry : variableMap.entrySet())
    {
      local = local.replace(entry.getKey(), entry.getValue());
    }
    
    return local;
  }
}
