package com.sft.core.general;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 *
 * @author development
 */
public class Cast 
{
  public static <T> List<T> toList(Object object, Class<T> clazz)
  {
    if (object == null)
      return null;
    
    if (object instanceof List<?>)
    {
      List<T> result = new ArrayList<>();

      for (Object o : (List<?>) object)      
        result.add(clazz.cast(o));
        
      return result;
    }

    throw new ClassCastException();
  }
  
  public static <T> Set<T> toSet(Object object, Class<T> clazz)
  {
    if (object == null)
      return null;
    
    if (object instanceof Set<?>)
    {
      Set<T> result = new HashSet<>();

      for (Object o : (Set<?>) object)      
        result.add(clazz.cast(o));
        
      return result;
    }

    throw new ClassCastException();
  }
  
  public static <K, T, S> Set<S> toSet(Map<K, T> map, K key, Class<S> clazz)
  {
    Object obj = map.get(key);
    if (obj == null)
      return null;
    
    if (obj instanceof Set<?>)
    {
      Set<S> result = new HashSet<>();

      for (Object o : (Set<?>) obj)
        result.add(clazz.cast(o));
        
      map.put(key, (T)result);
      
      return result;
    }
    
    throw new ClassCastException();
  }
  
  public static <K, V> Map<K, V> toMap(Object object, Class<K> keyType,
    Class<V> valueType)
  {
    if (object == null)
      return null;

    if (object instanceof Map<?, ?>)
    {
      Map<K, V> result = new HashMap<>();

      for (Map.Entry<?, ?> entry : ((Map<?, ?>) object).entrySet())
      {
        result.put(keyType.cast(entry.getKey()),
          valueType.cast(entry.getValue()));
      }

      return result;
    }

    throw new ClassCastException();
  }

  public static <K, V> List<Map<K, V>> toMapList(Object object,
    Class<K> keyType, Class<V> valueType)
  {
    if (object == null)
      return null;
      
    if (object instanceof List<?>)
    {
      List<Map<K, V>> result = new ArrayList<>();

      for (Object o : (List<?>) object)
      {
        result.add(toMap(o, keyType, valueType));
      }

      return result;
    }

    throw new ClassCastException();
  }
}
