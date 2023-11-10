package com.sft.core.stream.modifier;

import com.sft.core.exception.CoreException;
import com.sft.core.stream.StreamPacket;
import java.nio.charset.CharacterCodingException;

/**
 *
 * @author Administrador
 */
public abstract class StreamModifier 
{
  protected String header = "";
  
  public static StreamModifier resolveStreamModifier(String classname) throws CoreException 
  {
    try
    {
      ClassLoader classLoader = StreamModifier.class.getClassLoader();
      Class streammodifierClass;

      streammodifierClass = classLoader.loadClass(classname);

      return (StreamModifier)streammodifierClass.getDeclaredConstructor().newInstance();
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
  
  public abstract void preModifier(StreamPacket streamPacket) throws CharacterCodingException;
  public abstract void postModifier(StreamPacket streamPacket);
  
  public String getHeader() {
    return header;
  }

  public void setHeader(String header) {
    this.header = header;
  }
}
