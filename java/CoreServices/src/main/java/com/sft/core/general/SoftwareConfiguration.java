package com.sft.core.general;

/**
 *
 * @author David Ricardo
 */
public enum SoftwareConfiguration 
{
  RELEASE("Release"),
  DEBUG("Debug");
  
  private final String value;
  
  SoftwareConfiguration(String v) 
  {
    this.value = v;
  }

  public String getValue()
  {
    return value;
  }

  public static SoftwareConfiguration fromString(String v)
  {
    if (v != null)
    {
      for (SoftwareConfiguration b : SoftwareConfiguration.values())
      {
        if (v.equalsIgnoreCase(b.value))
        {
          return b;
        }
      }
    }
    return null;
  }
}
