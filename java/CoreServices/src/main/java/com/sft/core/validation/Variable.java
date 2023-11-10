package com.sft.core.validation;

public class Variable 
{
  protected Variable()
  {
  }
  
  public static final boolean isNumeric(String str)
  {
    if(str == null)
      return false;
    
    for (int i=0; i<str.length(); i++)
    {
      if (!Character.isDigit(str.charAt(i)))
        return false;
    }
    return true;
  }
  
  public static final boolean isDouble(String str)
  {
    if(str == null)
      return false;
    
    try
    {
      Double.parseDouble(str);
      return true;
    }
    catch(NumberFormatException e)
    {
      return false;
    }
  }
  
  public static final boolean isAlpha(String str)
  {
    if(str == null)
      return false;
    
    for (int i=0; i<str.length(); i++)
    {
      char c = str.charAt(i);
      if (!Character.isLetter(c))
        return false;
    }
    return true;
  }
  
  public static final boolean isAlphanumeric(String str)
  {
    if(str == null)
      return false;
    
    for (int i=0; i<str.length(); i++)
    {
      char c = str.charAt(i);
      if (!Character.isDigit(c) && !Character.isLetter(c))
        return false;
    }
    return true;
  }
  
  public static final boolean isAlphanumericSpecial(String str)
  {
    if(str == null)
      return false;
    
    for (int i=0; i<str.length(); i++)
    {
      char c = str.charAt(i);
      if (!Character.isDigit(c) && !Character.isLetter(c) && !isPunct(c))
        return false;
    }
    return true;
  }
  
  public static final boolean isPunct(Character c)
  {
    // depends on the Unicode/ASCII character sets...
    return ((c >= '!' && c <= '/') ||
            (c >= ':' && c <= '@') ||
            (c >= '[' && c <= '`') ||
            (c >= '{' && c <= '~'));
  }
}
