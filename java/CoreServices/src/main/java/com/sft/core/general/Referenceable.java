package com.sft.core.general;

/**
 *
 * @author David Ricardo
 * @param <T>
 */
public interface Referenceable<T>
{
  public T getIndex();
  public void setIndex(T index);
}
