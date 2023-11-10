package com.sft.core.general;

public class ClassUtils
{
  private ClassUtils()
  {
    throw new AssertionError();
  }
 // returns the class (without the package if any)
  public static String getClassName(Class c) 
  {
    String fqclassname = c.getName();
    int firstChar;
    firstChar = fqclassname.lastIndexOf ('.') + 1;
    if ( firstChar > 0 ) {
      fqclassname = fqclassname.substring ( firstChar );
      }
    return fqclassname;
  }

  public static String getClassName(String classname) 
  {
    int firstChar;
    String fqclassname = classname;
    firstChar = fqclassname.lastIndexOf ('.') + 1;
    if ( firstChar > 0 ) {
      fqclassname = fqclassname.substring ( firstChar );
      }
    return fqclassname;
  }

  // returns package and class name
  public static String getFullClassName(Class c) 
  {
    return  c.getName();
  }

  // returns the package without the classname, empty string if
  // there is no package
  public static String getPackageName(Class c) 
  {
    String fullyQualifiedName = c.getName();
    int lastDot = fullyQualifiedName.lastIndexOf ('.');
    if (lastDot==-1){ return ""; }
    return fullyQualifiedName.substring (0, lastDot);
  }
  
  public static String getPackageName(String className) 
  {
    String fullyQualifiedName = className;
    int lastDot = fullyQualifiedName.lastIndexOf ('.');
    if (lastDot==-1){ return ""; }
    return fullyQualifiedName.substring (0, lastDot);
  }
}
