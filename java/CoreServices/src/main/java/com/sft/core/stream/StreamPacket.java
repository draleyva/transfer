package com.sft.core.stream;

import com.sft.core.converter.ByteConvert;
import com.sft.core.exception.CoreException;
import com.sft.core.stream.modifier.StreamModifier;
import com.sft.log.StreamDirection;
import java.awt.event.KeyEvent;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;

public class StreamPacket implements IStreamPacketOptions
{
  private static int MAXSIZE = 65536;
  
  public static final int DEFAULT = 0;
  public static final int ENCODE = 1;
  public static final int DECODE = 2;
  
  protected static final String [] OPERATION = { "DEFAULT", "ENCODE", "DECODE" };
  
  private static StreamModifier streamModifier;
  
  static final byte PACKETBYTE = 0x2E;
  
  private byte [] sourceBuffer;
  private byte [] convertBuffer;
  private byte [] tempBuffer;
  
  private Charset streamCharSet;
  private Charset byteCharSet;
  private int avoidLength = 0;
  private boolean binary = false;
  private String sourceString;
  private String convertString;
  private String sourcePrintString;
  private String convertPrintString;
  private String sourceDetailString;
  private String convertDetailString;
  private int convertSize;
  private int sourceSize;
  private int connectionId = 0;
  private long messageId = 0;
  CharsetDecoder streamDecoder;
  CharsetEncoder streamEncoder;
  CharsetDecoder byteDecoder;
  CharsetEncoder byteEncoder;
  protected StreamDirection direction;
  int currentOperation = DEFAULT;

  public StreamPacket()
  {
    setStreamCharSet(Charset.defaultCharset());
    setByteCharSet(Charset.defaultCharset());
    
    sourceBuffer = new byte[MAXSIZE];
    convertBuffer = new byte[MAXSIZE*2];
    tempBuffer = new byte[MAXSIZE*2];
  }
  
  public StreamPacket(Charset streamcharset, Charset bytecharset)
  {
    setStreamCharSet(streamcharset);
    setByteCharSet(bytecharset);
    
    streamDecoder = streamCharSet.newDecoder();
    streamEncoder = streamCharSet.newEncoder();
    byteDecoder = byteCharSet.newDecoder();
    byteEncoder = byteCharSet.newEncoder();
    
    sourceBuffer = new byte[MAXSIZE];
    convertBuffer = new byte[MAXSIZE*2];
    tempBuffer = new byte[MAXSIZE*2];
  }
  
  public StreamPacket(IStreamPacketOptions options)
  {
    setStreamCharSet(options.getStreamCharSet());
    setByteCharSet(options.getByteCharSet());
    setAvoidLength(options.getAvoidLength());

    streamDecoder = streamCharSet.newDecoder();
    streamEncoder = streamCharSet.newEncoder();
    byteDecoder = byteCharSet.newDecoder();
    byteEncoder = byteCharSet.newEncoder();

    sourceBuffer = new byte[MAXSIZE];
    convertBuffer = new byte[MAXSIZE*2];
    tempBuffer = new byte[MAXSIZE*2];
  }
  
  public StreamPacket(Charset streamcharset, Charset bytecharset, int avoidlength)
  {
    setStreamCharSet(streamcharset);
    setByteCharSet(bytecharset);
    
    setAvoidLength(avoidlength);

    streamDecoder = streamCharSet.newDecoder();
    streamEncoder = streamCharSet.newEncoder();
    byteDecoder = byteCharSet.newDecoder();
    byteEncoder = byteCharSet.newEncoder();

    sourceBuffer = new byte[MAXSIZE];
    convertBuffer = new byte[MAXSIZE*2];
    tempBuffer = new byte[MAXSIZE*2];
  }
  
  public void release()
  {
    sourceBuffer = null;
    convertBuffer = null;
    tempBuffer = null;
  }
  
  public static void setStreamSize(int size)
  {
    MAXSIZE = size;
  }
  
  public static int getStreamSize()
  {
    return MAXSIZE;
  }
  
  public void copyTo(StreamPacket target) throws CoreException
  {
    target.convert(getSourceBuffer(), getSourceSize(), DEFAULT);
    target.setMessageId(messageId);
  }
      
  public int getCurrentOperation()
  {
    return currentOperation;
  }
  
  public String convertSubstring(int start, int length)
  {
    return ByteConvert.bytes2String(convertBuffer, start, length);
  }       
  
  public void replaceConvert(int convertedstart, int sourcestart, int length)
  {
    int i;
    for(i = 0; i < length; i++)
    {
      convertBuffer[convertedstart+i] = sourceBuffer[sourcestart+i];
    }
  }
  
  public String decodeSource(int start, int length) throws CharacterCodingException
  {
    int i;
    StringBuilder stringBuffer = new StringBuilder();
    CharBuffer decodebuffer = streamDecoder.decode(ByteBuffer.wrap(sourceBuffer, start, length));
    for(i = 0; i < decodebuffer.length(); i++ )
    {
      stringBuffer.append(decodebuffer.charAt(i));
    }
          
    return stringBuffer.toString();
  }
  
  public void replaceTemp(int start, int length, byte value)
  {
    int i;
    for(i = 0; i < length; i++)
    {
      tempBuffer[start+i] = value;
    }
  }
  
  public boolean isBinary()
  {
    return binary;
  }
  
  public void setBinary(boolean bin)
  {
    binary = bin;
  }

  public byte[] getSourceBuffer()
  {
    return sourceBuffer;
  }
  
  public byte[] getConvertBuffer()
  {
    return convertBuffer;
  }
  
  public String getSourceString()
  {
    return sourceString;
  }
  
  public String getConvertString() 
  {
    return convertString;
  }
      
  public byte[] getSourceBytes()
  {
    ByteBuffer bytearray = ByteBuffer.wrap(sourceBuffer, 0, sourceSize);

    return bytearray.array();
  }
  
  public byte[] getConvertBytes()
  {
    ByteBuffer bytearray = ByteBuffer.wrap(convertBuffer, 0, convertSize);

    return bytearray.array();
  }
  
  public int getConvertSize()
  {
    return convertSize;
  }
  
  public boolean isConvertEmpty()
  {
    return convertSize == 0;
  }
  
  public int getSourceSize()
  {
    return sourceSize;
  }
  
  public boolean isPrintableChar( char c ) 
  {
    Character.UnicodeBlock block = Character.UnicodeBlock.of( c );
    return (!Character.isISOControl(c)) &&
            c != KeyEvent.CHAR_UNDEFINED &&
            block != null &&
            block != Character.UnicodeBlock.SPECIALS;
  }
  
  public String convert(String string) throws CoreException
  {
    return convert(string, DEFAULT);
  }
  
  public String convert(String string, int operation) throws CoreException
  {
    byte [] stringbytes;
    int i;

    if(string == null)
    {
      return null;
    }
    
    sourceSize = string.length();
    stringbytes = string.getBytes();

    for(i = 0; i < sourceSize/*convertSize*/; i++)
    {
      sourceBuffer[i] = stringbytes[i];
    }

    return convert(operation);
  }

  private String convert(int operation) throws CoreException
  {
    int i;
    boolean sourcebinary = false;

    binary = false;
    sourceDetailString = "";
    sourcePrintString = "";
    
    convertDetailString = "";
    convertPrintString = "";
    
    sourceString = "";
    convertString = "";
    
    for(i = 0; i < sourceSize; i++ )
    {
      tempBuffer[i] = sourceBuffer[i];
    }

    if((byteCharSet!= null && streamCharSet != null) && !streamCharSet.name().equals(byteCharSet.name()) && sourceSize > avoidLength)
    {
      currentOperation = operation;

      try
      {
        switch(currentOperation)
        {
          case ENCODE:
						if(streamModifier != null)
							streamModifier.preModifier(this);

            CharBuffer decodebuffer = byteDecoder.decode(ByteBuffer.wrap(tempBuffer, 0, sourceSize));
            ByteBuffer eeb = streamEncoder.encode(decodebuffer);
            eeb.clear();
            eeb.get(convertBuffer, 0, eeb.limit());
            convertSize = eeb.limit();
          break;
          case DECODE:      
            streamModifier.preModifier(this);

            String sourcestring = ByteConvert.bytes2String(tempBuffer, sourceSize, streamCharSet.name());
            
            ByteBuffer deb = byteEncoder.encode(CharBuffer.wrap(sourcestring));
            deb.clear();
            deb.get(convertBuffer, 0, deb.limit());
            convertSize = deb.limit();
          break;
          default:
            currentOperation = DEFAULT;
          break;
        }
      }
      catch(Exception e)
      {
        throw new CoreException(e);
      }
    }
    else if(getStreamCharSet() != null && !Charset.defaultCharset().name().equals(streamCharSet.name()) && sourceSize > avoidLength)
    { 
      currentOperation = operation;

      try
      {
        switch(currentOperation)
        {
          case ENCODE:
						if(streamModifier != null)
							streamModifier.preModifier(this);
						
            String sourcestring = ByteConvert.bytes2String(tempBuffer, sourceSize);
            ByteBuffer encodebuffer = streamEncoder.encode(CharBuffer.wrap(sourcestring));
            encodebuffer.clear();
            encodebuffer.get(convertBuffer, 0, encodebuffer.limit());
            convertSize = encodebuffer.limit();
          break;
          case DECODE:
						if(streamModifier != null)
							streamModifier.preModifier(this);

            CharBuffer decodebuffer = streamDecoder.decode(ByteBuffer.wrap(tempBuffer, 0, sourceSize));
            for(i = 0; i < decodebuffer.length(); i++ )
            {
              convertBuffer[i] = (byte)decodebuffer.charAt(i);
              convertSize = decodebuffer.length();
            }
          break;
          default:
            currentOperation = DEFAULT;
          break;
        }
      }
      catch(Exception e)
      {
        throw new CoreException(e);
      }
    }
    else if(getByteCharSet() != null && !Charset.defaultCharset().name().equals(byteCharSet.name()) && sourceSize > avoidLength)
    {
      currentOperation = operation;

      try
      {
        switch(currentOperation)
        {
          case ENCODE:
						if(streamModifier != null)
							streamModifier.preModifier(this);

            CharBuffer decodebuffer = byteDecoder.decode(ByteBuffer.wrap(tempBuffer, 0, sourceSize));
            for(i = 0; i < decodebuffer.length(); i++ )
            {
              convertBuffer[i] = (byte)decodebuffer.charAt(i);
            }
            convertSize = decodebuffer.length();
          break;
          case DECODE:      
            streamModifier.preModifier(this);

            String sourcestring = ByteConvert.bytes2String(tempBuffer, sourceSize, Charset.defaultCharset().name());
            ByteBuffer encodebuffer = byteEncoder.encode(CharBuffer.wrap(sourcestring));
            encodebuffer.clear();
            encodebuffer.get(convertBuffer, 0, encodebuffer.limit());
            convertSize = encodebuffer.limit();
          break;
          default:
            currentOperation = DEFAULT;
          break;
        }
      }
      catch(Exception e)
      {
        throw new CoreException(e);
      }
    }
    else 
    {
      currentOperation = DEFAULT;
    }
    
    if(currentOperation == DEFAULT)
    {
      for(i = 0; i < sourceSize; i++ )
      {
        convertBuffer[i] = sourceBuffer[i];
      }
			convertSize = sourceSize;
    }
    
    // Modificador de stream
		if(streamModifier != null)
			streamModifier.postModifier(this);
    
    for(i = 0; i < sourceSize; i++ )
    {
      if(sourceBuffer[i] == 0 || sourceBuffer[i] == 10 || sourceBuffer[i] == 13 || !isPrintableChar((char)sourceBuffer[i]))
      {
        sourcebinary = true;
        break;
      }
    }
    
    for(i = 0; i < convertSize; i++ )
    {
      if(convertBuffer[i] == 0 || convertBuffer[i] == 10 || convertBuffer[i] == 13 || !isPrintableChar((char)convertBuffer[i]))
      {
        binary = true;
        break;
      }
    }

    if(!binary)
    {
      if(getByteCharSet() != null && !Charset.defaultCharset().name().equals(byteCharSet.name()) && sourceSize > avoidLength)
      {
        convertString = ByteConvert.bytes2String(convertBuffer, convertSize, byteCharSet.name());
      }
      else
      {
        convertString = ByteConvert.bytes2String(convertBuffer, convertSize);  
      } 
      
      sourceString = ByteConvert.bytes2String(sourceBuffer, sourceSize);
      
      if(currentOperation == DEFAULT)
      {
        return sourceString;
      }
    }
    else
    {
      sourceString = ByteConvert.byteArrayToHexString(sourceBuffer, sourceSize);
      convertString = ByteConvert.byteArrayToHexString(convertBuffer, convertSize);
    }
    
    StringBuilder sourcePrintBuilder = new StringBuilder();
    StringBuilder sourceDetailBuilder = new StringBuilder();
    StringBuilder convertPrintBuilder = new StringBuilder();
    StringBuilder convertDetailBuilder = new StringBuilder();

    if(currentOperation == DEFAULT)
    {
      for(i = 0; i < sourceSize; i++ )
      {
        sourceDetailBuilder.append("{").append(i);
        
        if(sourceBuffer[i] == 0 || sourceBuffer[i] == 10 || sourceBuffer[i] == 13)
        {        
          sourcePrintBuilder.append((char) PACKETBYTE);
          sourceDetailBuilder.append(".").append(sourceBuffer[i]);
        }
        else
        {
          sourcePrintBuilder.append((char) sourceBuffer[i]);
          sourceDetailBuilder.append(":").append(sourceBuffer[i]);
        }
        
        sourceDetailBuilder.append("}");
      }
      
      for(i = 0; i < convertSize; i++ )
      {
        convertDetailBuilder.append("{").append(i);
        
        if(convertBuffer[i] == 0 || convertBuffer[i] == 10 || convertBuffer[i] == 13)
        {
          convertPrintBuilder.append((char) PACKETBYTE);
          convertDetailBuilder.append(".").append(convertBuffer[i]);
        }
        else
        {
          convertPrintBuilder.append((char) convertBuffer[i]);
          convertDetailBuilder.append(":").append(convertBuffer[i]);
        }
        
        convertDetailBuilder.append("}");
      }
    }
    else
    {
      for(i = 0; i < sourceSize; i++ )
      {
        sourceDetailBuilder.append("{").append(i);

        if(sourceBuffer[i] == 0 || sourceBuffer[i] == 10 || sourceBuffer[i] == 13 || !isPrintableChar((char)sourceBuffer[i]))
        {
          sourcePrintBuilder.append((char) PACKETBYTE);
        }
        else
        {
          sourcePrintBuilder.append((char) sourceBuffer[i]);
        }

        if(sourcebinary)
        {
          sourceDetailBuilder.append(".").append(String.format("%x", sourceBuffer[i]));
        }
        else
        {
          sourceDetailBuilder.append(":").append((char)sourceBuffer[i]);
        }
        sourceDetailBuilder.append("}");
      }
      
      for(i = 0; i < convertSize; i++ )
      {
        convertDetailBuilder.append("{").append(i);
        if(convertBuffer[i] == 0 || convertBuffer[i] == 10 || convertBuffer[i] == 13 || !isPrintableChar((char)convertBuffer[i]))
        {
          convertPrintBuilder.append((char) PACKETBYTE);
        }
        else
        {
          convertPrintBuilder.append((char) convertBuffer[i]);
        }
        
        if(binary)
        {
          convertDetailBuilder.append(".").append(String.format("%x", convertBuffer[i]));
        }
        else
        {
          convertDetailBuilder.append(":").append((char)convertBuffer[i]);
        }

        convertDetailBuilder.append("}");
      }
    }
    
    sourceDetailString = sourceDetailBuilder.toString();
    sourcePrintString = sourcePrintBuilder.toString();
    
    convertDetailString = convertDetailBuilder.toString();
    convertPrintString = convertPrintBuilder.toString();

    return sourceString;
  }
  
  public String convert(byte [] array) throws CoreException
  {
    return convert(array, array.length, DEFAULT);
  }
  
  public String convert(byte [] array, int arraysize) throws CoreException
  {
    return convert(array, arraysize, DEFAULT);
  }
          
  public String convert(byte [] array, int arraysize, int operation) throws CoreException
  {
    ByteBuffer bytebuffer;
    sourceSize = arraysize;
    
    bytebuffer = ByteBuffer.wrap(array);
    bytebuffer.get(this.sourceBuffer, 0, sourceSize);
    
    return convert(operation);
  }
  
  public String convert(ByteBuffer bytebuffer) throws CoreException
  {
    return convert(bytebuffer, DEFAULT);
  }
  
  public String convert(ByteBuffer bytebuffer, int operation) throws CoreException
  {
    sourceSize = bytebuffer.limit();
    
    bytebuffer.get(this.sourceBuffer, 0, sourceSize);

    return convert(operation);
  }

  public String convert(ByteBuffer bytebuffer, int offset, int length) throws CoreException 
  {
    return convert(bytebuffer, offset, length, DEFAULT);
  }
  
  public String convert(ByteBuffer bytebuffer, int offset, int length, int operation) throws CoreException 
  {
    sourceSize = length;

    bytebuffer.get(this.sourceBuffer, offset, sourceSize);

    return convert(operation);
  }

  public long getMessageId()
  {
    return messageId;
  }

  public void setMessageId(long messageId)
  {
    this.messageId = messageId;
  }

  public String getSourcePrintString() {
    return sourcePrintString;
  }

  public String getSourceDetailString() {
    return sourceDetailString;
  }

  public String getConvertPrintString() {
    return convertPrintString;
  }

  public String getConvertDetailString() {
    return convertDetailString;
  }

  public static StreamModifier getStreamModifier() {
    return streamModifier;
  }

  public static void setStreamModifier(StreamModifier aStreamModifier) {
    streamModifier = aStreamModifier;
  }
  
  @Override
  public int getAvoidLength() {
    return avoidLength;
  }

  @Override
  public final void setAvoidLength(int avoidLength) {
    this.avoidLength = avoidLength;
  }
  
  /**
   * @return the streamCharSet
   */
  @Override
  public Charset getStreamCharSet() {
    return streamCharSet;
  }

  /**
   * @param streamCharSet the streamCharSet to set
   */
  @Override
  public final void setStreamCharSet(Charset streamCharSet) {
    this.streamCharSet = streamCharSet;
  }

  /**
   * @return the byteCharSet
   */
  @Override
  public Charset getByteCharSet() {
    return byteCharSet;
  }

  /**
   * @param byteCharSet the byteCharSet to set
   */
  @Override
  public final void setByteCharSet(Charset byteCharSet) {
    this.byteCharSet = byteCharSet;
  }
  
  /**
   * @return the direction
   */
  public StreamDirection getDirection() {
    return direction;
  }

  /**
   * @param direction the direction to set
   */
  public void setDirection(StreamDirection direction) {
    this.direction = direction;
  }
  
   /**
   * @return the connectionId
   */
  public int getConnectionId() {
    return connectionId;
  }

  /**
   * @param connectionId the connectionId to set
   */
  public void setConnectionId(int connectionId) {
    this.connectionId = connectionId;
  }
}
