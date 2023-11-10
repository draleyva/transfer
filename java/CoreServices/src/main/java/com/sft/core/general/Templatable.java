package com.sft.core.general;

/**
 *
 * @author David Ricardo
 */
public interface Templatable
{
  public String base();
  public void base(String source);
  public String getSource();
  public void setSource(String source);
  public String getTarget();
  public void setTarget(String target);
  public String getTemplate();
  public void setValue(String value);
  public String getValue();
  public boolean isValueNull();
  public void setTemplate(String template);
  public Boolean getCustom();
  public void setCustom(Boolean custom);
}
