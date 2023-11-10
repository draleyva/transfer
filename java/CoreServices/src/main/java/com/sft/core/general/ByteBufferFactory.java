package com.sft.core.general;

import java.nio.ByteBuffer;
import org.apache.commons.pool2.BasePooledObjectFactory;
import org.apache.commons.pool2.PooledObject;
import org.apache.commons.pool2.impl.DefaultPooledObject;

/**
 *
 * @author David Ricardo
 */
public class ByteBufferFactory extends BasePooledObjectFactory<ByteBuffer> 
{
  int capacity;
  
  protected ByteBufferFactory(int capacity)
  {
    this.capacity = capacity;
  }
  
  public static ByteBufferFactory create(int capacity)
  {
    return new ByteBufferFactory(capacity);
  }
  
  @Override
  public ByteBuffer create()
  {
    return ByteBuffer.allocate(capacity);
  }

  @Override
  public PooledObject<ByteBuffer> wrap(ByteBuffer buffer) 
  {
    return new DefaultPooledObject<>(buffer);
  }

  @Override
  public void passivateObject(PooledObject<ByteBuffer> pooledObject)
  {
    pooledObject.getObject().clear();
  }
}
