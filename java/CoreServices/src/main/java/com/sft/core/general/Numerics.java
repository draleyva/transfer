package com.sft.core.general;

/**
 *
 * @author David Ricardo
 */
public class Numerics 
{
  protected Numerics()
  {
  }
  
  public static Long nullToZero(Long l)
  {
    if(l == null)
      return 0L;
    
    return l;
  }
  
  public static Integer nullToZero(Integer i)
  {
    if(i == null)
      return 0;
    
    return i;
  }
  
  public static Double nullToZero(Double d)
  {
    if(d == null)
      return 0.0;
    
    return d;
  }
}
