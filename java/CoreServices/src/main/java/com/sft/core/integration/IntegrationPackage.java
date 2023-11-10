package com.sft.core.integration;

import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.sft.core.general.Format;
import com.sft.core.general.Type;
import com.sft.core.integration.exception.IntegrationDoubleFormatException;
import com.sft.core.integration.exception.IntegrationException;
import com.sft.core.integration.exception.IntegrationNumericFormatException;
import com.sft.core.validation.Variable;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

public class IntegrationPackage implements Serializable
{
  static final long serialVersionUID = 1000000888800000001L;
  public static final String INVALID_ELEMENT_NAME = "Invalid element name";

  private Map information;
  private Integer result;
  private String message;
  private String function;
  private Integer messageId;

  public IntegrationPackage()
  {
    information = new HashMap<>();
  }

  public IntegrationPackage(Integer result, String message)
  {
    information = new HashMap<String, String>();
    this.result = result;
    this.message = message;
  }

  public void addElement(IntegrationElement element, String value)
  {
    Preconditions.checkNotNull(element, "Invalid element");

    Preconditions.checkNotNull(Strings.emptyToNull(element.getElementName()), INVALID_ELEMENT_NAME);

    if(!element.isOptional())
    {
      Preconditions.checkNotNull(Strings.nullToEmpty(value), "Value is mandatory");
    }

    information.put(element.getElementName(), value);
  }

  public void addElement(String elementname, String value)
  {
    Preconditions.checkNotNull(Strings.emptyToNull(elementname), INVALID_ELEMENT_NAME);

    information.put(elementname, value);
  }

  public void addElement(String elementname, Integer value)
  {
    Preconditions.checkNotNull(Strings.emptyToNull(elementname), INVALID_ELEMENT_NAME);

    information.put(elementname, value.toString());
  }

  public String getElement(String elementname) throws IntegrationException
  {
    return getElement(elementname, null);
  }

  public String getElement(String elementname, String defval) throws IntegrationException
  {
    return getElement(elementname, defval, Type.STRING, Format.ALL, 0, Integer.MAX_VALUE);
  }

  public String getElement(String elementname, Format format, int min, int max) throws IntegrationException
  {
    return getElement(elementname, null, Type.STRING, format, min, max);
  }

  public static void validateElement(String elementname, String ev, Type type, Format format, int min, int max) throws IntegrationException
  {
    switch(type)
    {
      default:
      case STRING:
        switch(format)
        {
          case NUMERIC:
            if(!Variable.isNumeric(ev))
              throw new IntegrationNumericFormatException(elementname);
          break;
          case DOUBLE:
            if(!Variable.isDouble(ev))
              throw new IntegrationDoubleFormatException(elementname);
          break;
          case ALPHA:
            if(!Variable.isAlpha(ev))
              throw new IntegrationException(elementname+" invalid alpha format");
          break;
          case ALPHANUMERIC:
            if(!Variable.isAlphanumeric(ev))
              throw new IntegrationException(elementname+" invalid alphanumeric format");
          break;
          default:
          case ALPHANUMERICSPECIAL:
            if(!Variable.isAlphanumericSpecial(ev))
              throw new IntegrationException(elementname+" invalid alphanumeric/special format");
          break;
        }
      break;
      case INTEGER:
        if(!Variable.isNumeric(ev))
          throw new IntegrationNumericFormatException(elementname);
      break;
      case DOUBLE:
        if(!Variable.isDouble(ev))
          throw new IntegrationDoubleFormatException(elementname);
      break;
    }

    // Length validation
    if(ev != null)
    {
      int length = ev.length();

      if(length < min)
        throw new IntegrationException(elementname+" invalid minimum length : "+min);

      if(length > max)
        throw new IntegrationException(elementname+" invalid max length : "+max);
    }
  }

  public String getElement(String elementname, String defval, Type type, Format format, int min, int max) throws IntegrationException
  {
    Preconditions.checkNotNull(Strings.emptyToNull(elementname), INVALID_ELEMENT_NAME);

    String ev = (String)information.get(elementname);

    if(Strings.isNullOrEmpty(ev) && defval != null)
    {
      ev = defval;
    }

    validateElement(elementname, ev, type, format, min, max);

    return ev;
  }

  /**
   * @return the result
   */
  public Integer getResult() {
    return result;
  }

  /**
   * @param result the result to set
   */
  public void setResult(Integer result) {
    this.result = result;
  }

  /**
   * @return the message
   */
  public String getMessage() {
    return message;
  }

  /**
   * @param message the message to set
   */
  public void setMessage(String message) {
    this.message = message;
  }

  /**
   * @return the function
   */
  public String getFunction() {
    return function;
  }

  /**
   * @param function the function to set
   */
  public void setFunction(String function) {
    this.function = function;
  }

  /**
   * @return the information
   */
  public Map getInformation() {
    return information;
  }

  /**
   * @param information the information to set
   */
  public void setInformation(Map information) {
    this.information = information;
  }

  /**
   * @return the messageId
   */
  public Integer getMessageId() {
    return messageId;
  }

  /**
   * @param messageId the messageId to set
   */
  public void setMessageId(Integer messageId) {
    this.messageId = messageId;
  }
}
