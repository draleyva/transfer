package com.sft.core.integration.exception;

/**
 *
 * @author David Ricardo
 */
public class IntegrationNumericFormatException extends IntegrationException
{
  public IntegrationNumericFormatException(String element)
  {
    super(element + " invalid numeric format");
  }
}
