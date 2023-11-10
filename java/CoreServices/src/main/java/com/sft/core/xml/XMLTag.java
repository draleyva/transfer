package com.sft.core.xml;

import java.util.regex.Pattern;

public class XMLTag
{
  private String name;
  private String tag;
  private String endTag;
  private String emptyTag;
  private Pattern emptyTagPatten;
  private Integer startSize;
  private Integer endSize;
  private Integer emptySize;

  /**
   * @return the name
   */
  public String getName() {
    return name;
  }

  /**
   * @param name the name to set
   */
  public void setName(String name) {
    this.name = name;
  }

  /**
   * @return the tag
   */
  public String getTag() {
    return tag;
  }

  /**
   * @param tag the tag to set
   */
  public void setTag(String tag) {
    this.tag = tag;
  }

  /**
   * @return the startSize
   */
  public Integer getStartSize() {
    return startSize;
  }

  /**
   * @param startSize the startSize to set
   */
  public void setStartSize(Integer startSize) {
    this.startSize = startSize;
  }

  /**
   * @return the endSize
   */
  public Integer getEndSize() {
    return endSize;
  }

  /**
   * @param endSize the endSize to set
   */
  public void setEndSize(Integer endSize) {
    this.endSize = endSize;
  }

  /**
   * @return the emptyTag
   */
  public String getEmptyTag() {
    return emptyTag;
  }

  /**
   * @param emptyTag the emptyTag to set
   */
  public void setEmptyTag(String emptyTag) {
    this.emptyTag = emptyTag;
  }

  /**
   * @return the endTag
   */
  public String getEndTag() {
    return endTag;
  }

  /**
   * @param endTag the endTag to set
   */
  public void setEndTag(String endTag) {
    this.endTag = endTag;
  }

  /**
   *
   * @return the emptySize
   */
  public Integer getEmptySize()
  {
    return emptySize;
  }

  /**
   *
   * @param emptySize the emptySize to set
   */
  public void setEmptySize(Integer emptySize)
  {
    this.emptySize = emptySize;
  }

  /**
   * @return the emptyTagPatten
   */
  public Pattern getEmptyTagPatten() {
    return emptyTagPatten;
  }

  /**
   * @param emptyTagPatten the emptyTagPatten to set
   */
  public void setEmptyTagPatten(Pattern emptyTagPatten) {
    this.emptyTagPatten = emptyTagPatten;
  }
}
