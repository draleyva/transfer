package com.sft.core.exception;

/**
 *
 * @author David Ricardo
 */
public class CoreException extends Exception
{
  public CoreException(String message)
  {
    super(message);
  }
  
  public CoreException(Throwable cause)
  {
    super(cause);
  }
  
  public CoreException(String message, Throwable cause)
  {
    super(message, cause);
  }
}
