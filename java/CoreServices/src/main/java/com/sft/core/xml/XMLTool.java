package com.sft.core.xml;

import ch.qos.logback.classic.Logger;
import com.google.common.base.Strings;
import com.sft.core.general.Util;
import java.util.regex.Pattern;

public class XMLTool 
{
  protected static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(XMLTool.class);
  
  public static final String XML_HEADER_UTF8 = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";  
  public static final String EMPTY = "";
  
  protected XMLTool()
  {
    
  }
  
  public static String getValue(String buffer, String stringpath)
  {
    if(Strings.isNullOrEmpty(buffer))
      return null;
    
    String [] path = stringpath.split("/");
    int i;
    int nameIndex = 0;
    
    for(i = 0; i < path.length; i++)
    {
      nameIndex = buffer.indexOf("<"+path[i]+">", nameIndex);
    }
    
    if(nameIndex < 0)
      return null;
    
    int greaterThan = buffer.indexOf('>', nameIndex);
    if(greaterThan < 0)
      return null;
    
    int lessThan = buffer.indexOf('<', greaterThan);
    if(lessThan < 0)
      return null;

    return buffer.substring(greaterThan+1,lessThan);
  }
  
  public static String getValue(String buffer, int index, String start, int startsize, String end)
  {
    if(Strings.isNullOrEmpty(buffer))
      return null;
    
    int greaterThan = buffer.indexOf(start, index);
    if(greaterThan < 0)
      return null;
    
    int lessThan = buffer.indexOf(end, index+startsize);
    if(lessThan < 0)
      return null;

    return buffer.substring(greaterThan+startsize,lessThan);
  }
  
  public static XMLValue getValue(String buffer, String start, int startsize, String end, int endsize, Pattern emptyre, int emptysize)
  {
    XMLValue value = new XMLValue();
    
    if(Strings.isNullOrEmpty(buffer))
      return value;
    
    int greaterThan = buffer.indexOf(start);
    if(greaterThan < 0)
    {
      greaterThan = Util.indexOf(emptyre, buffer);
      
      if(greaterThan > 0)
      {
        value.setOffsetTagValue(emptysize, greaterThan, EMPTY);
      }
      
      return value;
    }
    else
    {
      int emptyIndex = Util.indexOf(emptyre, buffer);
      if(emptyIndex > 0 && emptyIndex < greaterThan)
      {
        value.setOffsetTagValue(emptysize, emptyIndex, EMPTY);

        return value;
      }
    }

    int lessThan = buffer.indexOf(end, startsize);
    
    if(lessThan < 0 || (greaterThan+startsize) > lessThan)
      return value;

    try
    {
      String content = buffer.substring(greaterThan+startsize, lessThan);
      
      value.setOffsetTagValue(startsize+endsize+content.length(), greaterThan, content);
    }
    catch(Exception e)
    {
      LOGGER.warn("getValue->", e);
    }
    
    return value;
  }

  public static String createTag(String tag)
  {
    StringBuilder builder = new StringBuilder();
    
    builder.append("<").append(tag).append(">");
    
    return builder.toString();
  }
  
  public static String createEndTag(String tag)
  {
    StringBuilder builder = new StringBuilder();
    
    builder.append("</").append(tag).append(">");
    
    return builder.toString();
  }
  
  public static String createEmptyTag(String tag)
  {
    StringBuilder builder = new StringBuilder();
    
    builder.append("<").append(tag).append("/>");
    
    return builder.toString();
  }
  
   public static Pattern createEmptyTagPattern(String tag)
  {
    StringBuilder builder = new StringBuilder();
    
    // "\\<LOCATION_PHONE\\s?+/\\s?+>"
    builder.append("\\<").append(tag).append("\\s?+/\\s?+>");
    
    return Pattern.compile(builder.toString());
  }
}
