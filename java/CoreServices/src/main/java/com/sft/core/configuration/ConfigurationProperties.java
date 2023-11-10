package com.sft.core.configuration;

import static com.sft.core.configuration.ConfigurationProperties.SOURCE.PROPERTIES;
import com.sft.log.LogUtil;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Properties;
import org.apache.commons.configuration2.Configuration;
import org.apache.commons.configuration2.ConfigurationDecoder;
import org.apache.commons.configuration2.ImmutableConfiguration;
import org.apache.commons.configuration2.MapConfiguration;
import org.apache.commons.configuration2.builder.fluent.Configurations;
import org.apache.commons.configuration2.interpol.ConfigurationInterpolator;
import org.apache.commons.configuration2.interpol.Lookup;
import org.apache.commons.configuration2.sync.LockMode;
import org.apache.commons.configuration2.sync.Synchronizer;

// https://commons.apache.org/proper/commons-configuration/userguide/quick_start.html
// https://commons.apache.org/proper/commons-configuration/userguide/howto_properties.html

public class ConfigurationProperties implements Configuration
{
  public enum SOURCE{PROPERTIES};
  protected Configuration config;
  protected SOURCE source;
  
  public static final Configurations CONFIGURATIONS = new Configurations();
  
  static
  {
    LogUtil.disableLog(new String[]{
      "org.apache.commons.beanutils.converters.ArrayConverter", 
      "org.apache.commons.beanutils.converters.ByteConverter",
      "org.apache.commons.beanutils.converters.BooleanConverter",
      "org.apache.commons.beanutils.converters.CharacterConverter",
      "org.apache.commons.beanutils.converters.DoubleConverter",
      "org.apache.commons.beanutils.converters.FloatConverter",
      "org.apache.commons.beanutils.converters.IntegerConverter",
      "org.apache.commons.beanutils.converters.LongConverter",
      "org.apache.commons.beanutils.converters.ShortConverter",
      "org.apache.commons.beanutils.converters.BigDecimalConverter",  
      "org.apache.commons.beanutils.converters.StringConverter",
      "org.apache.commons.beanutils.converters.BigIntegerConverter"});
  }
  
  public ConfigurationProperties(Map<String, Object> map)
  {
    source = PROPERTIES;
    config = new MapConfiguration(map);
  }
  
  public ConfigurationProperties()
  {
    this(new HashMap<String, Object>());
  }
  
  public ConfigurationProperties(SOURCE source)
  {
    this.source = source;
  }
  
  public Map toMap()
  {
    switch(source)
    {
      case PROPERTIES:
      {
        Map<String, Object> map = new HashMap<>();
        Iterator<String> keys = config.getKeys();
        String key;
        while(keys.hasNext()) 
        {
          key = keys.next();
          map.put(key, config.getProperty(key));
        }
        
        return map;
      }
    }
    
    return null;
  }
  
  @SuppressWarnings("all")
  public void load(String path) throws ConfigurationPropertiesException
  {
    try
    {
      switch(source)
      {
        case PROPERTIES:
          config = CONFIGURATIONS.properties(path);
        break;
      }
    }
    catch(Exception e)
    {
      throw new ConfigurationPropertiesException(e);
    }
  }
  
  @Override
  public boolean getBoolean(String key)
  {
    return config.getBoolean(key);
  }

  @Override
  public boolean getBoolean(String key, boolean defaultValue)
  {
    return config.getBoolean(key, defaultValue);
  }

  @Override
  public Integer getInteger(String key, Integer defaultValue)
  {
    return config.getInteger(key, defaultValue);
  }

  @Override
  public long getLong(String key) 
  {
    return config.getLong(key);
  }

  @Override
  public Long getLong(String key, Long defaultValue)
  {
    return config.getLong(key, defaultValue);
  }

  @Override
  public String getString(String key)
  {
    return config.getString(key);
  }

  @Override
  public String getString(String key, String defaultValue)
  {
    return config.getString(key, defaultValue);
  }
  
  @Override
  public Configuration subset(String prefix)
  {
    return config.subset(prefix);
  }

  @Override
  public void addProperty(String key, Object value)
  {
    config.addProperty(key, value);
  }

  @Override
  public void setProperty(String key, Object value)
  {
    config.setProperty(key, value);
  }

  @Override
  public void clearProperty(String key)
  {
    config.clearProperty(key);
  }

  @Override
  public void clear()
  {
    config.clear();
  }

  @Override
  public ConfigurationInterpolator getInterpolator()
  {
    return config.getInterpolator();
  }

  @Override
  public void setInterpolator(ConfigurationInterpolator ci)
  {
    config.setInterpolator(ci);
  }

  @Override
  public void installInterpolator(Map<String, ? extends Lookup> prefixLookups, Collection<? extends Lookup> defLookups)
  {
    config.installInterpolator(prefixLookups, defLookups);
  }

  @Override
  public boolean isEmpty()
  {
    return config.isEmpty();
  }

  @Override
  public int size()
  {
    return config.size();
  }

  @Override
  public boolean containsKey(String key)
  {
    return config.containsKey(key);
  }

  @Override
  public Object getProperty(String key)
  {
    return config.getProperties(key);
  }

  @Override
  public Iterator<String> getKeys(String prefix) 
  {
    return config.getKeys(prefix);
  }

  @Override
  public Iterator<String> getKeys()
  {
    return config.getKeys();
  }

  @Override
  public Properties getProperties(String key)
  {
    return config.getProperties(key);
  }

  @Override
  public Boolean getBoolean(String key, Boolean defaultValue)
  {
    return config.getBoolean(key, defaultValue);
  }

  @Override
  public byte getByte(String key)
  {
    return config.getByte(key);
  }

  @Override
  public byte getByte(String key, byte defaultValue)
  {
    return config.getByte(key, defaultValue);
  }

  @Override
  public Byte getByte(String key, Byte defaultValue)
  {
    return config.getByte(key, defaultValue);
  }

  @Override
  public double getDouble(String key)
  {
    return config.getDouble(key);
  }

  @Override
  public double getDouble(String key, double defaultValue)
  {
    return config.getDouble(key, defaultValue);
  }

  @Override
  public Double getDouble(String key, Double defaultValue)
  {
    return config.getDouble(key, defaultValue);
  }

  @Override
  public float getFloat(String key)
  {
    return config.getFloat(key);
  }

  @Override
  public float getFloat(String key, float defaultValue)
  {
    return config.getFloat(key, defaultValue);
  }

  @Override
  public Float getFloat(String key, Float defaultValue)
  {
    return config.getFloat(key, defaultValue);
  }

  @Override
  public int getInt(String key)
  {
    return config.getInt(key);
  }

  @Override
  public int getInt(String key, int defaultValue)
  {
    return config.getInt(key, defaultValue);
  }

  @Override
  public long getLong(String key, long defaultValue)
  {
    return config.getLong(key, defaultValue);
  }

  @Override
  public short getShort(String key)
  {
    return config.getShort(key);
  }

  @Override
  public short getShort(String key, short defaultValue)
  {
    return config.getShort(key, defaultValue);
  }

  @Override
  public Short getShort(String key, Short defaultValue)
  {
    return config.getShort(key, defaultValue);
  }

  @Override
  public BigDecimal getBigDecimal(String key)
  {
    return config.getBigDecimal(key);
  }

  @Override
  public BigDecimal getBigDecimal(String key, BigDecimal defaultValue)
  {
    return config.getBigDecimal(key, defaultValue);
  }

  @Override
  public BigInteger getBigInteger(String key)
  {
    return config.getBigInteger(key);
  }

  @Override
  public BigInteger getBigInteger(String key, BigInteger defaultValue)
  {
    return config.getBigInteger(key, defaultValue);
  }

  @Override
  public String getEncodedString(String key, ConfigurationDecoder decoder)
  {
    return config.getEncodedString(key, decoder);
  }

  @Override
  public String getEncodedString(String key)
  {
    return config.getEncodedString(key);
  }

  @Override
  public String[] getStringArray(String key)
  {
    return config.getStringArray(key);
  }

  @Override
  public List<Object> getList(String key)
  {
    return config.getList(key);
  }

  @Override
  public List<Object> getList(String key, List<?> defaultValue)
  {
    return config.getList(key, defaultValue);
  }

  @Override
  public <T> T get(Class<T> cls, String key)
  {
    return config.get(cls, key);
  }

  @Override
  public <T> T get(Class<T> cls, String key, T defaultValue)
  {
    return config.get(cls, key, defaultValue);
  }

  @Override
  public Object getArray(Class<?> cls, String key)
  {
    return config.getArray(cls, key);
  }

  @Deprecated
  @Override
  public Object getArray(Class<?> cls, String key, Object defaultValue)
  {
    return config.getArray(cls, key, defaultValue);
  }

  @Override
  public <T> List<T> getList(Class<T> cls, String key)
  {
    return config.getList(cls, key);
  }

  @Override
  public <T> List<T> getList(Class<T> cls, String key, List<T> defaultValue)
  {
    return config.getList(cls, key, defaultValue);
  }

  @Override
  public <T> Collection<T> getCollection(Class<T> cls, String key, Collection<T> target) 
  {
    return config.getCollection(cls, key, target);
  }

  @Override
  public <T> Collection<T> getCollection(Class<T> cls, String key, Collection<T> target, Collection<T> defaultValue)
  {
    return config.getCollection(cls, key, target, defaultValue);
  }

  @Override
  public ImmutableConfiguration immutableSubset(String prefix)
  {
    return config.immutableSubset(prefix);
  }

  @Override
  public Synchronizer getSynchronizer()
  {
    return config.getSynchronizer();
  }

  @Override
  public void setSynchronizer(Synchronizer sync)
  {
    config.setSynchronizer(sync);
  }

  @Override
  public void lock(LockMode mode)
  {
    config.lock(mode);
  }

  @Override
  public void unlock(LockMode mode)
  {
    config.unlock(mode);
  }
}
