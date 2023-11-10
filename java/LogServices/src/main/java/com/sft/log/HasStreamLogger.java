package com.sft.log;

/**
 *
 * @author David Ricardo
 */
public interface HasStreamLogger 
{
  public void strm(long tick, boolean converted, Object... object);
}
