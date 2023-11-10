package com.sft.core.general;

import ch.qos.logback.classic.Logger;
import com.sft.core.general.exception.ClassLoaderException;
import com.sft.core.general.exception.ConstructorException;
import com.sft.core.general.exception.InstanceException;
import com.sft.core.general.exception.InvokeException;
import com.sft.core.general.exception.MethodException;
import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 *
 * @author David Ricardo Leyva
 */
public class ClassLoaderFactory<T>
{
  public static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(ClassLoaderFactory.class);
  
  static final Map<String, ClassLoader> CLASSLOADERMAP = new ConcurrentHashMap<>();
  static final Map<String, Class> CLASSMAP = new ConcurrentHashMap<>();
  static final Map<String, String> CLASSNAMEMAP = new ConcurrentHashMap<>();
  static final Map<String, ClassLoaderException> FAILMAP = new ConcurrentHashMap<>();
  
  Class classMember;
  Map<String, Method> methodMap = new HashMap<>();
  Constructor<T> constructor;
          
  protected ClassLoaderFactory(Class c)
  {
    classMember = c;
  }
  
  public static final ClassLoaderFactory load(Class parent, String classname) throws ClassLoaderException
  {
    // check name in the exception map to automatic resolve
    if(FAILMAP.containsKey(classname))
      throw FAILMAP.get(classname);
    
    try
    {
      if(!CLASSNAMEMAP.containsKey(classname))
      {
        String cn;

        if(classname.contains("."))
          cn = classname;
        else     
          cn = parent.getPackage().getName()+"."+classname;
        
        CLASSLOADERMAP.put(cn, parent.getClassLoader());
        
        CLASSMAP.put(cn, CLASSLOADERMAP.get(cn).loadClass(cn));
        
        CLASSNAMEMAP.put(classname, cn);
      }

      return new ClassLoaderFactory(CLASSMAP.get(CLASSNAMEMAP.get(classname)));
    }
    catch(Throwable e)
    {
      ClassLoaderException cle = new ClassLoaderException(e);
      ClassLoaderException oldvalue = FAILMAP.put(classname, cle);
      if(oldvalue != null)
        LOGGER.error("loadClass->", oldvalue);
      throw cle;
    }
  }
  
  public final ClassLoaderFactory get(String name, Class<?>... parameterTypes) throws MethodException
  {
    try
    {
      methodMap.put(name, classMember.getMethod(name, parameterTypes));
      
      return this;
    }
    catch(Throwable e)
    {
      throw new MethodException(e);
    }
  }
  
  public final ClassLoaderFactory constructor(Class<?>... parameterTypes) throws ConstructorException
  {
    try
    {
      constructor = classMember.getConstructor(parameterTypes);
      return this;
    }
    catch(Throwable e)
    {
      throw new ConstructorException(e);
    }
  }
  
  public Object invoke(String name, Object obj, Object... args) throws InvokeException
  {
    try
    {
      return methodMap.get(name).invoke(obj, args);
    }
    catch(Throwable e)
    {
      throw new InvokeException(e);
    }
  }
  
  public T newInstance(Object ... initargs) throws InstanceException
  {
    try
    {
      return constructor.newInstance(initargs);
    }
    catch(Throwable e)
    {
      throw new InstanceException(e);
    }
  }
}
