package com.sft.core.general;

import com.google.common.base.CharMatcher;
import com.google.common.base.Splitter;
import com.sft.core.exception.CoreException;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 *
 * @author David Ricardo
 */
public class DecoderEncoder
{
  static final Splitter SPLITTER = Splitter.on(CharMatcher.anyOf("/\\|"));
  static final Map<String, Charset> CHARSETMAP = new ConcurrentHashMap<>();
  private final CharsetDecoder csDecoder;
  private final CharsetEncoder csEncoder;
  
  protected DecoderEncoder(CharsetDecoder csd, CharsetEncoder cse)
  {
    csDecoder = csd;
    csEncoder = cse;
  }
  
  public static final DecoderEncoder create(String decoderencoder) throws CoreException
  {
    List<String> list = SPLITTER.splitToList(decoderencoder);
    return create(list.get(0), list.get(1));
  }
  
  public static final DecoderEncoder create(String decoder, String encoder) throws CoreException
  {
    if(!CHARSETMAP.containsKey(decoder))
      CHARSETMAP.put(decoder, Charset.forName(decoder));
    if(!CHARSETMAP.containsKey(encoder))
      CHARSETMAP.put(encoder, Charset.forName(encoder));
    
    return new DecoderEncoder(CHARSETMAP.get(decoder).newDecoder(), CHARSETMAP.get(encoder).newEncoder());
  }
    
  public byte [] decodeEncode(byte [] array) throws CoreException
  {
    try
    {
      return csEncoder.encode(csDecoder.decode(ByteBuffer.wrap(array))).array();
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
  
  public byte [] encodeDecode(byte [] array) throws CoreException
  {
    try
    {
      CharBuffer cb = csDecoder.decode(csEncoder.encode(ByteBuffer.wrap(array).asCharBuffer()));
      ByteBuffer bb = ByteBuffer.allocate((int)(cb.length() * csEncoder.maxBytesPerChar()));
      CharBuffer converter = bb.asCharBuffer();
      converter.append(cb);

      byte [] result = new byte[bb.position()];
      bb.rewind();
      bb.get(result, 0, result.length);
      return result;
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
  
  /**
   * @return the csDecoder
   */
  public CharsetDecoder getDecoder() {
    return csDecoder;
  }

  /**
   * @return the csEncoder
   */
  public CharsetEncoder getEncoder() {
    return csEncoder;
  }
}
