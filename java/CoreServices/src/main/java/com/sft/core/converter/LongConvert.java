package com.sft.core.converter;

import com.google.common.base.Preconditions;

public class LongConvert 
{
   private static final char[] HEX = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
   
  protected LongConvert()
  {
  }

  public static long string2Long(String hexadecimal) throws NumberFormatException
  {
    char[] chars;
    char c;
    long value;
    int i;

    Preconditions.checkNotNull(hexadecimal);

    chars = hexadecimal.toUpperCase().toCharArray();
    Preconditions.checkArgument(chars.length == 16);
      
    value = 0;

    for (i = 0; i < 16; i++) 
    {
      c = chars[i];
      if (c >= '0' && c <= '9')
      {
        value = ((value << 4) | (0xff & (c - '0')));
      }
      else if (c >= 'A' && c <= 'F')
      {
        value = ((value << 4) | (0xff & (c - 'A' + 10)));
      }
      else
      {
        throw new NumberFormatException("Invalid hex character: " + c);
      }
    }

    return value;
  }

  public static String long2String(long value)
  {
    char[] hexs;
    int i;
    int c;

    hexs = new char[16];
    for (i = 0; i < 16; i++)
    {
      c = (int)(value & 0xf);
      hexs[16-i-1] = HEX[c];
      value = value >> 4;
    }
    
    return new String(hexs);
  }
  
  public static String longLong2String(long value1, long value2)
  {
    char[] hexs;
    int i;
    int c;

    hexs = new char[16];
    
    for (i = 0; i < 8; i++)
    {
      c = (int)(value1 & 0xf);
      hexs[8-i-1] = HEX[c];
      value1 = value1 >> 4;
      
      c = (int)(value2 & 0xf);
      hexs[16-i-1] = HEX[c];
      value2 = value2 >> 4;
    }
    
    return new String(hexs);
  }
}
