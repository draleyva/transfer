package com.sft.core.general.exception;

/**
 *
 * @author development
 */
public class ConstructorException extends Exception
{
  public ConstructorException()
  {
  }
  
  public ConstructorException(String message)
  {
    super(message);
  }
  
  public ConstructorException(Throwable cause)
  {
    super(cause);
  }
  
  public ConstructorException(String message, Throwable cause) 
  {
    super(message, cause);
  }
}

