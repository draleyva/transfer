package com.sft.core.stream;

import java.nio.charset.Charset;

/**
 *
 * @author David Ricardo
 */
public interface IStreamPacketOptions
{
  public Charset getStreamCharSet();
  public void setStreamCharSet(Charset streamCharSet);
  public Charset getByteCharSet();
  public void setByteCharSet(Charset byteCharSet);
  public int getAvoidLength();
  public void setAvoidLength(int avoidLength);
}
