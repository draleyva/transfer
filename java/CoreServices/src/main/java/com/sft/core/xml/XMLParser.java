package com.sft.core.xml;

import ch.qos.logback.classic.Logger;
import com.google.common.base.Preconditions;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

public class XMLParser
{
  protected static final Logger logger = (Logger)org.slf4j.LoggerFactory.getLogger(XMLParser.class);
  
  HashMap<String, XMLTag> tagMap;
  
  public XMLParser()
  {
    tagMap = new HashMap<>();
  }
  
  public Map<String, String> getMap(String buffer, String ... tags)
  {
    for(String tag : tags)
    {
      if(tagMap.containsKey(tag))
        continue;
      
      XMLTag xmltag = new XMLTag();
      
      xmltag.setName(tag);
      
      xmltag.setTag(XMLTool.createTag(tag));
      xmltag.setStartSize(tag.length()+2);
      
      xmltag.setEndTag(XMLTool.createEndTag(tag));
      xmltag.setEndSize(tag.length()+3);
      
      xmltag.setEmptyTag(XMLTool.createEmptyTag(tag));
      xmltag.setEmptySize(tag.length()+3);
      
      xmltag.setEmptyTagPatten(XMLTool.createEmptyTagPattern(tag));
      
      tagMap.put(tag, xmltag);
    }
    
    LinkedHashMap<String, String> hashmap = new LinkedHashMap<>();
    
    XMLTag xmltag;
    XMLValue value;
    
    for(String tag : tags)
    {
      xmltag = tagMap.get(tag);
      value = XMLTool.getValue(buffer, 
        xmltag.getTag(), xmltag.getStartSize(), xmltag.getEndTag(), xmltag.getEndSize(), 
        xmltag.getEmptyTagPatten(), xmltag.getEmptySize());
      
      hashmap.put(tag, value.getValue());
    }
    
    return hashmap;
  }
  
  public List<LinkedHashMap<String, String>> getMapList(String buffer, String ... tags)
  {
    Preconditions.checkNotNull(buffer);

    for(String tag : tags)
    {
      if(tagMap.containsKey(tag))
        continue;
      
      XMLTag xmltag = new XMLTag();
      
      xmltag.setName(tag);
      
      xmltag.setTag(XMLTool.createTag(tag));
      xmltag.setStartSize(tag.length()+2);
      
      xmltag.setEndTag(XMLTool.createEndTag(tag));
      xmltag.setEndSize(tag.length()+3);
      
      xmltag.setEmptyTag(XMLTool.createEmptyTag(tag));
      xmltag.setEmptySize(tag.length()+3);
      
      xmltag.setEmptyTagPatten(XMLTool.createEmptyTagPattern(tag));
      
      tagMap.put(tag, xmltag);
    }

    List<LinkedHashMap<String, String>> map = new ArrayList<>();
    XMLValue value;
    XMLTag xmltag;
    int tagcounter;
    int tagoffset;
    int tagsize = 0;
    
    String temporal = buffer;

    while(true)
    {
      LinkedHashMap<String, String> hashmap = new LinkedHashMap<>();
      tagcounter = 0;
      tagoffset = 0;

      for(String tag : tags)
      {
        xmltag = tagMap.get(tag);
        value = XMLTool.getValue(temporal, 
          xmltag.getTag(), xmltag.getStartSize(), xmltag.getEndTag(), xmltag.getEndSize(), 
          xmltag.getEmptyTagPatten(), xmltag.getEmptySize());

        // add value
        hashmap.put(tag, value.getValue());
        
        // Se ha encontrado un valor
        if(value.getOffset() > 0)
        {
          ++tagcounter;
          
          // Se guarda el maximo desplazamiento
          if(tagoffset < value.getTagOffset())
          {
            tagoffset = value.getTagOffset();
            tagsize = value.getOffset();
          }
        }
      }

      if (tagoffset+tagsize >= temporal.length() || tagcounter == 0)
      {
        break;
      }
      
      temporal = temporal.substring(tagoffset+tagsize);

      // si en una pasada no se encuentran tag se da por terminado

      map.add(hashmap);
    }
  
    return map;
  }
}
