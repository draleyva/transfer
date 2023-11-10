package com.sft.core.general;

/**
 *
 * @author DavidRicardo
 */
public enum Type 
{
  INTEGER("INTEGER", 0), DOUBLE("DOUBLE", 1), STRING("STRING", 2);
    
  private final int value;
  private final String key;
    
  Type(String key, int value)
  {
    this.value = value;
    this.key = key;
  }

  public final int getValue()
  {
    return value;
  }
  
  public final String getKey()
  {
    return key;
  }
}
