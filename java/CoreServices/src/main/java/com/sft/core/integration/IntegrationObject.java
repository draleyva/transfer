package com.sft.core.integration;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

/**
 *
 * @author David Ricardo
 */
public class IntegrationObject implements Serializable
{
  private HashMap map;
  protected byte [] buffer;

  public IntegrationObject()
  {
    map = new HashMap<>();
  }

  /**
   * @return the map
   */
  public Map getMap() {
    return map;
  }


  /**
   * @return the buffer
   */
  public byte[] getBuffer() {
    return buffer;
  }

  /**
   * @param buffer the buffer to set
   */
  public void setBuffer(byte[] buffer) {
    this.buffer = buffer;
  }
}
