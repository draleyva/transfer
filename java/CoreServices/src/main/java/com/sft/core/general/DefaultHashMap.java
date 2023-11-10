package com.sft.core.general;

import java.util.LinkedHashMap;

public class DefaultHashMap<K,V> extends LinkedHashMap<K,V> 
{
  public int getAsInteger(K key)
  {
    String value = get(key).toString();
    
    try
    {
      return Integer.parseInt(value);
    }
    catch(Exception e)
    {
      return -1;
    }
  }
  
  public int getAsInteger(K key, V defaultValue)
  {
    String value = getDefault(key, defaultValue).toString();
    
    try
    {
      return Integer.parseInt(value);
    }
    catch(Exception e)
    {
      return -1;
    }
  }
  
  public V getDefault(K key, V defaultValue)
  {
    if (containsKey(key))
    {
      return get(key);
    }

    return defaultValue;
  }
}
