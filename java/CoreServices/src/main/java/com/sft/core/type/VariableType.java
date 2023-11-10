package com.sft.core.type;

/**
 *
 * @author sanms
 */
public enum VariableType 
{
  STRING("S"), INTEGER("N"), DOUBLE("D"), DATE("DATE"), TIME("TIME");
  
  protected final String type;
    
  VariableType(String t)
  {
    type = t;
  }
  
  public static VariableType fromString(String s)
  {
    if (s != null)
    {
      for (VariableType b : VariableType.values())
      {
        if (s.equalsIgnoreCase(b.type))
        {
          return b;
        }
      }
    }
    
    return null;
  }
}
