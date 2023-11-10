package com.sft.core.general;

import com.google.common.base.Preconditions;
import com.google.common.base.Splitter;
import com.google.common.base.Strings;
import com.sft.core.exception.CoreException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

public class MapUtil
{
  public static final int MAPUTIL_FAIL = -99;
  public static final int MAPUTIL_L = -1;
  public static final int MAPUTIL_LL = -2;
  public static final int MAPUTIL_LLL = -3;
  public static final int MAPUTIL_LLLL = -4;
  static final int MAPUTIL_FIELDSIZE = 4;
  static final String MAPUTIL_TAGSEPARATOR = ";";
  static final Splitter MAPUTIL_TAGSPLITTER = Splitter.on(MAPUTIL_TAGSEPARATOR);
  
  protected MapUtil()
  {
  }
  
  public static final boolean isNullOrEmpty(Map<String, ?> map, String key) 
  {
    Object value = map.get(key);
    return Strings.isNullOrEmpty(value == null ? null : value.toString());
  }
  
	public static Map <String, String> getMap(String string, String pairseparator, String keyvalueseparator)
	{
		Preconditions.checkArgument(!Strings.isNullOrEmpty(pairseparator));
		Preconditions.checkArgument(!Strings.isNullOrEmpty(keyvalueseparator));
		
		Map <String, String> map = new LinkedHashMap<>();
		
		Splitter pairsplittter = Splitter.on(pairseparator);
		Splitter keyvaluesplittter = Splitter.on(keyvalueseparator);
		
		Iterator<String> pairitor = pairsplittter.split(string).iterator();
		Iterator<String> keyvalueitor;
		int index;
		
		String pair;
		String key;
		String value;
		
		while(pairitor.hasNext())
		{
			pair = pairitor.next();
			if(Strings.isNullOrEmpty(pair))
				continue;
			
			keyvalueitor = keyvaluesplittter.split(pair).iterator();
			
			index = 0;
		
			key = null;
			value = null;
			
			while(keyvalueitor.hasNext())
			{
				switch(index)
				{
					case 0:
						key = keyvalueitor.next();
					break;
					case 1:
						value = keyvalueitor.next();
					break;
          default:
          break;
				}
				
				++index;
				
				if(index == 2)
				{
					map.put(key, value);
					break;
				}
			}
		}
		
		return map;
	}
  
  public static Map map2Table(Map map)
  {
    if(map == null)
      return null;
    
    Hashtable ht = new Hashtable();
    Entry entry;
    
    for(Object obj : map.entrySet())
    {
      entry = (Entry)obj;
      ht.put(entry.getKey(), entry.getValue());
    }
    
    return ht;
  }
  
  public static Map<String, String> getTable(String string, String pairseparator, String keyvalueseparator)
	{
		Preconditions.checkArgument(!Strings.isNullOrEmpty(pairseparator));
		Preconditions.checkArgument(!Strings.isNullOrEmpty(keyvalueseparator));
		
		Hashtable <String, String> map = new Hashtable <>();
		
		Splitter pairsplittter = Splitter.on(pairseparator);
		Splitter keyvaluesplittter = Splitter.on(keyvalueseparator);
		
		Iterator<String> pairitor = pairsplittter.split(string).iterator();
		Iterator<String> keyvalueitor;
		int index;
		
		String pair;
		String key;
		String value;
		
		while(pairitor.hasNext())
		{
			pair = pairitor.next();
			if(Strings.isNullOrEmpty(pair))
				continue;
			
			keyvalueitor = keyvaluesplittter.split(pair).iterator();
			
			index = 0;
		
			key = null;
			value = null;
			
			while(keyvalueitor.hasNext())
			{
				if(index == 0)
				{
          key = keyvalueitor.next();
        }
        else if (index == 1)
        {
          value = keyvalueitor.next();
        }
				
				++index;
				
				if(index == 2)
				{
					map.put(key, value);
					break;
				}
			}
		}
		
		return map;
	}

  public static int getFieldSize(String s, int offset)
  {
    return getFieldSize(s, offset, MAPUTIL_FIELDSIZE);
  }
  
  public static int getFieldSize(String s, int offset, int ss)
  {
    String var;
    
    try
    {
      var = s.substring(offset, offset+ss);
    }
    catch(Exception e)
    {
      return MAPUTIL_FAIL;
    }
    
    switch (var) 
    {
      case "000L":
      case "L000":
        return MAPUTIL_L;
      case "00LL":
      case "LL00":
        return MAPUTIL_LL;
      case "0LLL":
      case "LLL0":
        return MAPUTIL_LLL;
      case "LLLL":
        return MAPUTIL_LLLL;
      default:
        break;
    }
    
    try
    {
      return Integer.parseInt(var);
    }
    catch(NumberFormatException e)
    {
      return MAPUTIL_FAIL;
    }
  }
  
  public static String getString(String s, int offset, int width)
  {
    try
    {
      return s.substring(offset, offset+width);
    }
    catch(Exception e)
    {
      return "";
    }
  }
  
  public static List<DefaultHashMap<String, String>> stream2Map(String stream, int offset, String format, String tags) throws CoreException
  {
    // Numero de campos
    int columns;
    int [] columnswidth;
    String [] columntag;
    int i;
    int cw;
    String var;
    List<DefaultHashMap<String, String>> result;
      
    Preconditions.checkArgument(!Strings.isNullOrEmpty(format));
    
    columns = format.length()/MAPUTIL_FIELDSIZE;
    
    // Se crea la cantidad de columnas segun la data
    columnswidth = new int[columns];
    columntag = new String[columns];
    
    // Verificar que el menos la cantidad de columnas no exceda a la de tags
    Iterator<String> itor = MAPUTIL_TAGSPLITTER.split(tags).iterator();
    
    for(i = 0; i < columns; i++)
    {
      if(!itor.hasNext())
      {
        throw new CoreException(String.format("Format %s is not consistent with tags %s", format, tags));
      }
      
      columntag[i] = itor.next();
    }
    
    int formatoffset = 0;
    
    for(i = 0; i < columns; i++)
    {
      cw = getFieldSize(format, formatoffset);
      
      if(cw == MAPUTIL_FAIL)
        throw new CoreException("Invalid format");
      
      if(cw == 0)
        throw new CoreException("Invalid field size");
      
      formatoffset += MAPUTIL_FIELDSIZE;
      columnswidth[i] = cw;
    }
    
    int sizevalue;
    int fieldsize;
    result = new ArrayList<>();
    
    while(stream.length() - offset > 0)
    {
      DefaultHashMap<String, String> dhm = new DefaultHashMap<>();
      
      for(i = 0; i < columns; i++)
      {
        switch(columnswidth[i])
        {
          case MAPUTIL_L:
          case MAPUTIL_LL:
          case MAPUTIL_LLL:
          case MAPUTIL_LLLL:
            sizevalue = columnswidth[i]*-1;
            fieldsize = getFieldSize(stream, offset, sizevalue);
            offset += sizevalue;
          break;
          default:
            fieldsize = columnswidth[i];
          break;
        }
        
        var = getString(stream, offset, fieldsize);
        offset += fieldsize;
        
        dhm.put(columntag[i], var);
      }
      
      result.add(dhm);
    }
      
    return result;
  }
}
