package com.sft.core.xml;

public class XMLValue 
{
  private String value;
  private int offset;
  private int tagOffset;
  private long elapse;
  
  public XMLValue()
  {
    value = null;
    offset = 0;
    tagOffset = 0;
  }
  
  public void setOffsetTagValue(int offset, int tag, String value)
  {
    this.offset = offset;
    this.tagOffset = tag;
    this.value = value;
  }
  
  /**
   * @return the value
   */
  public String getValue() {
    return value;
  }

  /**
   * @param value the value to set
   */
  public void setValue(String value) {
    this.value = value;
  }

  /**
   * @return the offset
   */
  public int getOffset() {
    return offset;
  }

  /**
   * @param offset the offset to set
   */
  public void setOffset(int offset) {
    this.offset = offset;
  }

  /**
   * @return the tagOffset
   */
  public int getTagOffset() {
    return tagOffset;
  }

  /**
   * @param tagOffset the tagOffset to set
   */
  public void setTagOffset(int tagOffset) {
    this.tagOffset = tagOffset;
  }

  /**
   * @return the elapse
   */
  public long getElapse() {
    return elapse;
  }

  /**
   * @param elapse the elapse to set
   */
  public void setElapse(long elapse) {
    this.elapse = elapse;
  }
}
