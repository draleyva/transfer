package com.sft.core.general;

public class Java 
{
  static final String VERSION = "java.version";
  static final String VMNAME = "java.vm.name";
  static final String VMVERSION = "java.vm.version";
  static final String VMVENDOR = "java.vm.vendor";
  static final String VMSPECIFICATIONVERSION = "java.vm.specification.version";
  static final String VMSPECIFICATIONNAME = "java.vm.specification.name";
  static final String VMSPECIFICATIONVENDOR = "java.vm.specification.vendor";
  static final String PATHSEPARATOR = "path.separator";
  
  protected Java()
  {
  }
  
  public static final String getPathSeparator()
  {
    return System.getProperty(PATHSEPARATOR);
  }
  
  public static final String getVersion()
  {
    return System.getProperty(VERSION);
  }
  
  public static final String getJVMName()
  {
    return System.getProperty(VMNAME);
  }
  
  public static final String getJVMVendor()
  {
    return System.getProperty(VMVENDOR);
  }
  
  public static final String getJVMVersion()
  {
    return System.getProperty(VMVERSION);
  }
  
  public static final String getJVMSpecificationVersion()
  {
    return System.getProperty(VMSPECIFICATIONVERSION);
  }
  
  public static final String getJVMSpecificationName()
  {
    return System.getProperty(VMSPECIFICATIONNAME);
  }
}
