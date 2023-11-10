package com.sft.core.integration;

public class IntegrationElement 
{
  // Nombre del dato
  private String name;
  // Longitud del dato
  private Integer size;
  private boolean optional;

  public IntegrationElement()
  {
    name = null;
    size = 0;
    optional = false;
  }
  
  public IntegrationElement(String en, Integer es)
  {
    name = en;
    size = es;
  }
  
  /**
   * @return the name
   */
  public String getElementName() {
    return name;
  }

  /**
   * @param elementName the name to set
   */
  public void setName(String elementName) {
    this.name = elementName;
  }

  /**
   * @return the size
   */
  public Integer getSize() {
    return size;
  }

  /**
   * @param elementSize the size to set
   */
  public void setSize(Integer elementSize) {
    this.size = elementSize;
  }

  /**
   * @return the optional
   */
  public boolean isOptional() {
    return optional;
  }

  /**
   * @param optional the optional to set
   */
  public void setOptional(boolean optional) {
    this.optional = optional;
  }
}
