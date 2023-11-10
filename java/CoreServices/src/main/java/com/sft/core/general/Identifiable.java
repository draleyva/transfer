package com.sft.core.general;

/**
 *
 * @author David Ricardo
 * @param <T>
 */
public interface Identifiable<T>
{
  public T getId();
  public void setId(T id);
}
