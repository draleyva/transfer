package com.sft.core.configuration;

import com.sft.core.service.ConfigSelector;
import java.nio.charset.Charset;

/**
 *
 * @author development
 */
public interface ConfigurationBase
{
  boolean isClustered();
  void setClustered(boolean aclustered);
  Charset getByteCharSet();
  public ConfigSelector security();
  public boolean hasSecurity();  
  public ConfigSelector cryptography();
  public boolean hasCryptography();
}
