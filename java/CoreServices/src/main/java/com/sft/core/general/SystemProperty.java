package com.sft.core.general;

import ch.qos.logback.classic.Logger;
import com.google.common.base.Optional;
import com.google.common.base.Strings;
import java.util.concurrent.ConcurrentHashMap;

public class SystemProperty 
{
  protected static Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(SystemProperty.class);
  static final ConcurrentHashMap<String, Optional<String>> CHM = new ConcurrentHashMap<>();
  
  private SystemProperty()
  { 
  }
  
  public static String get(String propertyname)
  {
    if(Strings.isNullOrEmpty(propertyname))
      return null;
    
    if(CHM.contains(propertyname))
      return CHM.get(propertyname).orNull();
      
    String envproperty;
    String property;
    
    try
    {
      envproperty = System.getenv(propertyname);
    }
    catch(Exception e)
    {
      envproperty = null;
    }

    if(envproperty != null)
      property = System.getProperty(propertyname, envproperty);
    else
      property = System.getProperty(propertyname);
    
    CHM.put(propertyname, Optional.fromNullable(property==null?envproperty:property));
    
    if(envproperty != null && CHM.get(propertyname).orNull() != null)
    {      
      if(!envproperty.equals(CHM.get(propertyname).get()))
      {
        LOGGER.info("Property [{}] has been overridden [{}]->[{}]", propertyname, property, CHM.get(propertyname).get());
      }
    }
    
    return CHM.get(propertyname).orNull();
  }
}
