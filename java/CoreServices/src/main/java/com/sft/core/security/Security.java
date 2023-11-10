package com.sft.core.security;

import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Security 
{
  protected Security()
  {
  }
  
  public static String createMAC(String data)
  {
    MessageDigest md = null;
    try
    {
      md = MessageDigest.getInstance("SHA-256");
    }
    catch(NoSuchAlgorithmException e)
    {
      return null;
      
    }

    return new BigInteger(1, md.digest(data.getBytes())).toString(16).substring(0, 16);
  }
}
