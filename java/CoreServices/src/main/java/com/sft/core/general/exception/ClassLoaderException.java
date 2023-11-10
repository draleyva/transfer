package com.sft.core.general.exception;

/**
 *
 * @author sanms
 */
public class ClassLoaderException extends Exception
{
  public ClassLoaderException()
  {
  }
  
  public ClassLoaderException(String message)
  {
    super(message);
  }
  
  public ClassLoaderException(Throwable cause)
  {
    super(cause);
  }
  
  public ClassLoaderException(String message, Throwable cause) 
  {
    super(message, cause);
  }
}
