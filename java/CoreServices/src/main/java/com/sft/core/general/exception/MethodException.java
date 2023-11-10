package com.sft.core.general.exception;

/**
 *
 * @author development
 */
public class MethodException extends Exception
{
  public MethodException()
  {
  }
  
  public MethodException(String message)
  {
    super(message);
  }
  
  public MethodException(Throwable cause)
  {
    super(cause);
  }
  
  public MethodException(String message, Throwable cause) 
  {
    super(message, cause);
  }
}
