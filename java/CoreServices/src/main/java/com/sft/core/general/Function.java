package com.sft.core.general;

import com.google.common.base.CharMatcher;
import com.google.common.base.Splitter;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

public class Function
{
  private String name;
  private List<String> parameter;
  static Splitter splitter = Splitter.on(CharMatcher.anyOf("),("));
  
  protected Function()
  {
    parameter = new ArrayList<>();
  }
  
  public static Function parseFunction(String function)
  {
    Function f = new Function();
    Iterator<String> itor = splitter.split(function).iterator();
    String value;
    int index = 0;
    
    while(itor.hasNext())
    {
      value = itor.next();
      
      if(index == 0)
      {
        f.setName(value);
      }
      else
      {
        f.getParameter().add(value);
      }
      
      ++index;
    }
    
    return f;
  }

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
   * @return the parameter
   */
  public List<String> getParameter() {
    return parameter;
  }

  /**
   * @param parameter the parameter to set
   */
  public void setParameter(List<String> parameter) {
    this.parameter = parameter;
  }
}
