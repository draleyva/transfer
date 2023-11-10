package com.sft.core.xml;

public class XMLBuilderUnit 
{
  private StringBuilder stringBuilder;
  private String iterator;
  private String expression;
  
  public XMLBuilderUnit()
  {
    iterator = "";
    expression = "";
    stringBuilder = new StringBuilder();
  }

  /**
   * @return the stringBuilder
   */
  public StringBuilder getStringBuilder() {
    return stringBuilder;
  }

  /**
   * @param stringBuilder the stringBuilder to set
   */
  public void setStringBuilder(StringBuilder stringBuilder) {
    this.stringBuilder = stringBuilder;
  }

  /**
   * @return the iterator
   */
  public String getIterator() {
    return iterator;
  }

  /**
   * @param iterator the iterator to set
   */
  public void setIterator(String iterator) {
    this.iterator = iterator;
  }

  /**
   * @return the expression
   */
  public String getExpression() {
    return expression;
  }

  /**
   * @param expression the expression to set
   */
  public void setExpression(String expression) {
    this.expression = expression;
  }
}
