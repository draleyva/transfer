package com.sft.core.integration.exception;

/**
 *
 * @author David Ricardo
 */
public class IntegrationDoubleFormatException extends IntegrationException
{
  public IntegrationDoubleFormatException(String element)
  {
    super(element + " invalid double format");
  }
}
