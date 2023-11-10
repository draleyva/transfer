package com.sft.core.general;

import com.sft.core.exception.CoreException;
import java.io.FileInputStream;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.Set;

public class Environment
{
  protected static final Map<String, String> ENVIRONMENTMAP = new HashMap<>();
  
  protected Environment()
  {  
  }
  
  public static void load(String filename) throws CoreException 
  {
    Properties prop = new Properties();
    
    try(FileInputStream is = new FileInputStream(filename))
    {        
      prop.load(is);
      Set<String> properties = prop.stringPropertyNames();
      
      for(String property : properties)
      {
        ENVIRONMENTMAP.put(property, prop.getProperty(property));
      }
      
      prop.clear();
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
}
