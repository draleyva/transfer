package com.sft.core.configuration;

public class ConfigurationPropertiesException extends Exception 
{
  static final long serialVersionUID = -1000000333300000001L;
  
  public ConfigurationPropertiesException(Throwable t)
  {
    super(t);
  }
  
  public ConfigurationPropertiesException(String s)
  {
    super(s);
  }
}