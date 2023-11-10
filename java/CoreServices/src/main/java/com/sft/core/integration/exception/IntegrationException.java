package com.sft.core.integration.exception;

public class IntegrationException extends Exception
{
  static final long serialVersionUID = -1000000333300010001L;
  
  private Integer exceptionCode;
  
  public IntegrationException(Throwable t)
  {
    super(t);
  }
  
  public IntegrationException(String s)
  {
    super(s);
    exceptionCode = -1;
  }
  
  public IntegrationException(Throwable t, Integer ec)
  {
    super(t);
    exceptionCode = ec;
  }
  
  public IntegrationException(String s, Integer ec)
  {
    super(s);
    exceptionCode = ec;
  }

  /**
   * @return the exceptionCode
   */
  public Integer getExceptionCode() {
    return exceptionCode;
  }

  /**
   * @param exceptionCode the exceptionCode to set
   */
  public void setExceptionCode(Integer exceptionCode) {
    this.exceptionCode = exceptionCode;
  }
}
