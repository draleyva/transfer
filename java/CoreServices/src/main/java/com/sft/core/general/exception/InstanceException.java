package com.sft.core.general.exception;

/**
 *
 * @author development
 */
public class InstanceException extends Exception
{
  public InstanceException()
  {
  }
  
  public InstanceException(String message)
  {
    super(message);
  }
  
  public InstanceException(Throwable cause)
  {
    super(cause);
  }
  
  public InstanceException(String message, Throwable cause) 
  {
    super(message, cause);
  }
}