package com.sft.core.runtime;

import com.sft.core.exception.CoreException;
import java.lang.reflect.Field;

public class LibraryPath 
{
  protected LibraryPath()
  {
  }

  /**
  * Sets the java library path to the specified path
  *
  * @param path the new library path
  * @throws Exception
  */
  
  public static String[] splitPaths(String separatedTexts) 
  {
    if(separatedTexts.indexOf(';') >= 0)
      return separatedTexts.split(";");
    else
      return separatedTexts.split(":");
  }

  private static Object resizeArray (Object oldArray, int newSize) 
  {
   int oldSize = java.lang.reflect.Array.getLength(oldArray);
   Class elementType = oldArray.getClass().getComponentType();
   Object newArray = java.lang.reflect.Array.newInstance(
         elementType,newSize);
   int preserveLength = Math.min(oldSize,newSize);
   if (preserveLength > 0)
      System.arraycopy (oldArray,0,newArray,0,preserveLength);
   return newArray; 
  }
  
  public static void setSystemLibraryPath(String path) throws CoreException 
  {
    System.setProperty("java.library.path", path);
 
    try
    {
      final Field sysPathsField = ClassLoader.class.getDeclaredField("sys_paths");
      sysPathsField.setAccessible(true);
      sysPathsField.set(null, null);
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
}
