package com.sft.core.pool.exception;

public class ObjectPoolException extends Exception
{
  static final long serialVersionUID = -1000000333300020001L;
  int errorCode;
  
  public ObjectPoolException(Throwable t)
  {
    super(t);
  }
  
  public ObjectPoolException(String s)
  {
    super(s);
  }
  
  public ObjectPoolException(Throwable t, Integer ec)
  {
    super(t);
    errorCode = ec;
  }
  
  public ObjectPoolException(String s, Integer ec)
  {
    super(s);
    errorCode = ec;
  }
}
