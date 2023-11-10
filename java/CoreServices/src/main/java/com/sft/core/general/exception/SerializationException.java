package com.sft.core.general.exception;

public class SerializationException extends Exception 
{
  static final long serialVersionUID = -1000000333300000005L;
  
  public SerializationException(Throwable t)
  {
    super(t);
  }
  
  public SerializationException(String s)
  {
    super(s);
  }
}