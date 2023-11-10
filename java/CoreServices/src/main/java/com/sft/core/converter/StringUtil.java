package com.sft.core.converter;

import com.google.common.base.Strings;
import java.math.BigInteger;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;

public class StringUtil 
{
  public static final Charset IBM_1047 = Charset.forName("IBM-1047");
  protected static final char[] HEX_CHARS = "0123456789abcdef".toCharArray();
  public static final String DECOMPOSE_REGEX = "\\p{InCombiningDiacriticalMarks}+";
  
  protected StringUtil()
  {
  }
  
  public static String padEnd(String s, int n) 
  {
    return Strings.padEnd(s, n, ' ');
  }

  public static String padStart(String s, int n)
  {
    return Strings.padStart(s, n, ' ');
  }
  
  public static String padEndExt(String string, int minLength, char padChar)
  {
    String value = Strings.nullToEmpty(string).trim();
    
    if(value.length() > minLength)
      value = value.substring(0, minLength);
    
    return Strings.padEnd(value, minLength, padChar);
  }
  
  public static String padStartExt(String string, int minLength, char padChar) 
  {
    String value = Strings.nullToEmpty(string).trim();
    
    if(value.length() > minLength)
      value = value.substring(0, minLength);
    
    return Strings.padStart(value, minLength, padChar);
  }
  
  public static String padStartEx(Long val, int minLength, char padChar) 
  {
    return padStartExt(val.toString(), minLength, padChar);
  }
  
  public static String removeChar(String s, char c) 
  {
    StringBuilder r = new StringBuilder();
    char cur;
    for (int i = 0; i < s.length(); i ++)
    {
      cur = s.charAt(i);
      if (cur != c) r.append( cur );
    }
    return r.toString();
  }
  
  public static boolean isEmpty(String obj)
  {
    return ((obj == null) || (obj.trim().length() == 0));
  }

  public static String convert8859toUTF8(String data)
  {
    if(data == null)
      return null;
    
    byte[] arrByte1 = data.getBytes(StandardCharsets.ISO_8859_1);
    byte[] arrByte2 = data.getBytes(StandardCharsets.UTF_8);
    String strE1;
    String strE2;

    strE1 = new String(arrByte1, StandardCharsets.UTF_8);
    strE2 = new String(arrByte2, StandardCharsets.UTF_8);

    return (arrByte1.length < arrByte2.length && strE1.length() > strE2.length())?strE1:strE2;
  }
	
	public static String uppercaseFirstLetters(String str) 
	{
		boolean prevWasWhiteSp = true;
		char[] chars = str.toCharArray();
		for (int i = 0; i < chars.length; i++)
		{
			if (Character.isLetter(chars[i]))
			{
				if (prevWasWhiteSp)
				{
					chars[i] = Character.toUpperCase(chars[i]);
				}
				prevWasWhiteSp = false;
			}
			else
			{
				prevWasWhiteSp = Character.isWhitespace(chars[i]);
			}
		}
		return new String(chars);
	}
  
  public static String decompose(String s, boolean force)
  {
    String result = java.text.Normalizer.normalize(s, java.text.Normalizer.Form.NFD).replaceAll(DECOMPOSE_REGEX,"");
    
    if(force)
    {
      result = result.replaceAll("[^\\x00-\\x7F]", " ");
    }
   
    return result;
  }
  
  public static String asHex(byte[] buf, int length)
  {
    char[] chars = new char[2 * length];
    for (int i = 0; i < length; ++i)
    {
      chars[2 * i] = HEX_CHARS[(buf[i] & 0xF0) >>> 4];
      chars[2 * i + 1] = HEX_CHARS[buf[i] & 0x0F];
    }
    return new String(chars);
  }
  
  public static String convert8859toIBM1047(String data)
  {
    byte[] res = data.getBytes(StandardCharsets.ISO_8859_1);
    String out = asHex(res, res.length);
    byte[] bytebuff = new BigInteger(out, 16).toByteArray();
    return new String(bytebuff, IBM_1047);
  }
  
  public static String convert8859toIBM1047(byte [] iso8859bytes, int length)
  {
    String out = asHex(iso8859bytes, length);
    byte[] bytebuff = new BigInteger(out, 16).toByteArray();
    return new String(bytebuff, IBM_1047);
  }
  
  public static String convert8859toIBM1047(byte [] iso8859bytes)
  {
    return convert8859toIBM1047(iso8859bytes, iso8859bytes.length);
  }
  
  public static String convertIBM1047to8859(String data)
  {
    byte[] result = data.getBytes(IBM_1047);
    String output = asHex(result, result.length);
    byte[] b = new BigInteger(output, 16).toByteArray();
    return new String(b, StandardCharsets.ISO_8859_1);
  }
  
  public static String convertIBM1047to8859(byte [] ebcdicbytes)
  {
    String output = asHex(ebcdicbytes, ebcdicbytes.length);
    byte[] b = new BigInteger(output, 16).toByteArray();
    return new String(b, StandardCharsets.ISO_8859_1);
  }  
}
