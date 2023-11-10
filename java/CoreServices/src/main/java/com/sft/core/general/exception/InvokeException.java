package com.sft.core.general.exception;

/**
 *
 * @author development
 */
public class InvokeException extends Exception
{
  public InvokeException()
  {
  }
  
  public InvokeException(String message)
  {
    super(message);
  }
  
  public InvokeException(Throwable cause)
  {
    super(cause);
  }
  
  public InvokeException(String message, Throwable cause) 
  {
    super(message, cause);
  }
}
