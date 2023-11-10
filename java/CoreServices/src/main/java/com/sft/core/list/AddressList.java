package com.sft.core.list;

import com.google.common.base.CharMatcher;
import com.google.common.base.Splitter;
import com.google.common.base.Strings;
import com.sft.core.exception.CoreException;
import com.sft.core.general.Util;
import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.List;
import java.util.Map.Entry;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

public class AddressList extends Thread
{
  private final HashMap<String, String> WL = new HashMap<>();
  private static final String DEFAULTVALUE = "permitted";
  private final ReadWriteLock rwlock = new ReentrantReadWriteLock();
  static final Splitter SPLITTER = Splitter.on(CharMatcher.anyOf(" \t,|")).omitEmptyStrings().trimResults();
  private File file;
  long lastModified;
  
  public AddressList()
  {
    lastModified = 0;
  }
  
  public void setup(String filename) throws CoreException
  {
    file = new File(filename);
    setup(file);
  }
  
  public void setup(File file) throws CoreException
  {     
    if(!file.canRead())
      throw new CoreException("Can not read");
    
    update();
    
    start();
  }
  
  @Override
  public void run()
  {
    while(true)
    {
      if(lastModified != file.lastModified())
      {
        lastModified = file.lastModified();
        update();
      }
      
      try
      {
        Thread.sleep(125);
      }
      catch (Exception ex)
      {
        // eat this exception
      }
    }
  }
  
  public void update()
  {
    List<String> temp;
    
    rwlock.writeLock().lock();

    try(BufferedReader br = new BufferedReader(new InputStreamReader(new DataInputStream(new FileInputStream(file)))))
    {
      WL.clear();

      String strLine;      
      //Read File Line By Line
      while ((strLine = br.readLine()) != null)
      {
        if(strLine.startsWith("#"))
          continue;
        
        temp = SPLITTER.splitToList(strLine);
        
        WL.put(temp.get(0), temp.get(1));
      }
    }
    catch (Exception e)
    {
      //Catch exception if any
    }
    finally
    {
      rwlock.writeLock().unlock();
    }
  }
  
  public boolean checkKey(String key, boolean ifempty)
  {
    boolean result = ifempty;
    
    rwlock.readLock().lock();
    try
    {
      if(!WL.isEmpty())
      {  
        result = WL.containsKey(key);
        if(!result)
        {
          String re;
          
          for(String k : WL.keySet())
          {
            re = Util.wildcardToRegex(k);
            if(key.matches(re))
            {
              result = true;
              break;
            }
          }
        }
      }
    }
    finally
    {
      rwlock.readLock().unlock();
    }
    
    return result;
  }
  
  public String getValue(String key)
  {
    String value;
            
    if(WL.isEmpty())
      return DEFAULTVALUE;
    
    value = WL.get(key);
    if(Strings.isNullOrEmpty(value))
    {
      String re;
      
      for(Entry<String, String> entry : WL.entrySet())
      {
        re = Util.wildcardToRegex(entry.getKey());
        if(key.matches(re))
        {
          value = entry.getValue();
          break;
        }
      }
    }
    
    return value == null?"":value;
  }
  
  public void release()
  {
    WL.clear();
  }
}
