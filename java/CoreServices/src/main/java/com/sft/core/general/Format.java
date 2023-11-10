package com.sft.core.general;

/**
 *
 * @author DavidRicardo
 */
public enum Format
{
  NUMERIC("NUMERIC", 0), DOUBLE("DOUBLE", 1), ALPHA("ALPHA", 2), ALPHANUMERIC("ALPHANUMERIC", 3), ALPHANUMERICSPECIAL("ALPHANUMERICSPECIAL", 4), ALL("ALL", 5);
    
  private final int value;
  private final String key;
    
  Format(String key, int value)
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
