package com.sft.core.general;

public class OperatingSystem 
{
  public static final String OSNAME = "os.name";
  public static final String OSVERSION = "os.version";
  public static final String OSARCHITECTURE = "os.arch";
  
  private static final String OS = getName().toLowerCase();
  
  protected OperatingSystem()
  {
  }
  
  public static String getName()
  {
    return System.getProperty(OSNAME);
  }
 
  public static String getVersion()
  {
    return System.getProperty(OSVERSION);
  }
  
  public static String getArchitecture()
  {
    return System.getProperty(OSARCHITECTURE);
  }
  
  public static boolean isWindows()
  {
		return (OS.contains("win"));
	}
 
	public static boolean isMac()
  {
		return OS.contains("mac");
	}
 
	public static boolean isUnix()
  {
		return OS.contains("nix") || OS.contains("nux") || OS.contains("aix");
	}
 
  public static boolean isLinux()
  {
		return OS.contains("nix") || OS.contains("nux");
	}
  
  public static boolean isAIX()
  {
		return OS.contains("aix");
	}
  
	public static boolean isSolaris()
  {
		return (OS.contains("sunos"));
	}
}
