package com.sft.core.type;

/**
 *
 * @author sanms
 */
public enum EvaluatorType
{
  FILENAME("FILENAME"), SUM("SUM"), COUNT("COUNT"), ENTITY("ENTITY"), MANDATORY("MANDATORY");
  
  protected final String type;
    
  EvaluatorType(String t)
  {
    type = t;
  }
  
  public static EvaluatorType fromString(String s)
  {
    if (s != null)
    {
      for (EvaluatorType b : EvaluatorType.values())
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
