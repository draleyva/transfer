package com.sft.core.service;

/**
 *
 * @author sanms
 */
public enum ConfigSelector
{
  SYSTEM("SYSTEM", 10),
  PRODUCT("PRODUCT", 11),
	INSTALLATION("INSTALLATION", 12),
  SERVICE("SERVICE", 13);
	
	private final String name;
	private final int id;
	
	private ConfigSelector(String s, int id)
  {
		this.name = s;
		this.id = id;
	}

	public String getName()
  {
		return name;
	}

	public int getId()
  {
		return id;
	}
  
  public static ConfigSelector fromInt(int id)
  {
    for (ConfigSelector b : ConfigSelector.values())
    {
      if (b.getId() == id)
        return b;
    }
    
    return null;
  }
}
