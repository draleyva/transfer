package com.sft.core.converter;

import com.sft.core.exception.CoreException;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;

public class ByteConvert
{
  private ByteConvert()
  {  
  }

  public static byte[] unpackByteArray(byte [] binarydata, int packetsize) throws CoreException
  {
    int size = Integer.parseInt(ByteConvert.bytes2String(binarydata, packetsize));
    
    byte [] bytearray = new byte[size];
    for(int byteidx = 0; byteidx < size; ++byteidx)
    {
      bytearray[byteidx] = (binarydata[packetsize+byteidx]);
    }
    
    return bytearray;
  }
  
  public static byte[] buildNewLengthByteArray(byte[] barrayIn, int newLength)
  {
    ByteBuffer barrayOutBuffer = ByteBuffer.allocate(newLength);
    barrayOutBuffer.put(barrayIn, 0, newLength);
    return barrayOutBuffer.array();
  }
  
  public static String byteArrayToHexString(byte[] b)
  {
    return byteArrayToHexString(b, b.length);
  }

  public static String byteArrayToHexString(byte[] b, int length)
  {
    StringBuilder sb = new StringBuilder(length * 2);
    for (int i = 0; i < length; i++)
    {
      int v = b[i] & 0xff;
      if (v < 16) {
        sb.append('0');
      }
      sb.append(Integer.toHexString(v));
    }
    return sb.toString().toUpperCase();
  }

  public static byte[] hexStringToByteArray(String s)
  {
    byte[] b = new byte[s.length() / 2];
    for (int i = 0; i < b.length; i++){
      int index = i * 2;
      int v = Integer.parseInt(s.substring(index, index + 2), 16);
      b[i] = (byte)v;
    }
    return b;
  }

  public static String bytes2String(byte[] bytes, Charset cs)
  {
    return new String(bytes, cs);
  }
  
  public static String bytes2String(byte[] bytes, String charsetName)
  {
    try 
    {
      return new String(bytes, charsetName);
    }
    catch (UnsupportedEncodingException e) 
    {
      return null;
    }
  }
  
  public static String bytes2String(byte[] bytes)
  {
    return bytes2String(bytes, bytes.length);
  }

  public static String bytes2String(byte[] bytes, int length)
  {
    return bytes2String(bytes, 0, length);
  }
  
  public static String bytes2String(byte[] bytes, int length, String encoding)
  {
    return bytes2String(bytes, 0, length, encoding);
  }
  
  public static String bytes2String(byte[] bytes, int start, int length)
  {
		return bytes2String(bytes, start, length, Charset.defaultCharset());
  }
  
  public static String bytes2String(byte[] bytes, int start, int length, String encoding)
  {
    try
    {
      return new String(bytes, start, length, encoding);
    }
    catch (Exception e) 
    {
      return null;
    }
  }
  
  public static String bytes2String(byte[] bytes, int start, int length, Charset charset)
  {
    try
    {
      return new String(bytes, start, length, charset);
    }
    catch (Exception e) 
    {
      return null;
    }
  }
  
  public static short byte2Short(byte [] bytes, int start)
  {
    ByteBuffer bb = ByteBuffer.wrap(bytes, start, 2);
    return bb.getShort();
  }
  
  public static int stringByte2Int(byte [] bytes, int start, int size)
  {
    try
    {
      return Integer.parseInt(new String(bytes, start, size));
    }
    catch(Exception e)
    {
      return -1;
    }
  }
  
  public static byte [] short2Byte(short num)
  {
    ByteBuffer buffer = ByteBuffer.allocate(2);
    buffer.putShort(num);
    return buffer.array();
  }
  
  public static byte[] appendData(short firstObject,byte[] secondObject)
  {
    byte[] byteArray = short2Byte(firstObject);
    
    return appendData(byteArray,secondObject);
  }
  
  public static byte[] appendData(byte firstObject,byte[] secondObject)
  {
    byte[] byteArray= {firstObject};
    return appendData(byteArray,secondObject);
  }

  public static byte[] appendData(byte[] firstObject,byte secondByte)
  {
    byte[] byteArray= {secondByte};
    return appendData(firstObject,byteArray);
  }

  public static byte[] appendData(byte[] firstObject,byte[] secondObject)
  {
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream( );
    try 
    {
      if (firstObject!=null && firstObject.length!=0)
          outputStream.write(firstObject);
      if (secondObject!=null && secondObject.length!=0)   
          outputStream.write(secondObject);
    }
    catch (IOException e)
    {
      return new byte[0];
    }
    return outputStream.toByteArray();
  }
  
  public static byte[] appendData(byte[] firstObject, byte[] secondObject, int start, int len)
  {
    ByteArrayOutputStream outputStream = new ByteArrayOutputStream( );
    try
    {
      if (firstObject!=null && firstObject.length!=0)
          outputStream.write(firstObject);
      if (secondObject!=null && secondObject.length!=0)   
          outputStream.write(secondObject, start, len);
    }
    catch (IOException e)
    {
      return new byte[0];
    }
    return outputStream.toByteArray();
  }
}
