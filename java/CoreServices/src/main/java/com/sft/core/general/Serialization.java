package com.sft.core.general;

import com.google.common.base.Preconditions;
import com.sft.core.general.exception.SerializationException;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import org.apache.commons.codec.binary.Base64;

public class Serialization
{
  protected Serialization() 
  {
    super();
  }
  
  public static Object clone(Serializable object) throws SerializationException
  {
    return deserialize(serialize(object));
  }
  
  public static Object zipClone(Serializable object) throws SerializationException
  {
    return zipDeserialize(zipSerialize(object));
  }
  
  public static void zipSerialize(Serializable obj, OutputStream outputStream) throws SerializationException
  {
    Preconditions.checkNotNull(outputStream);
    
    try(ObjectOutputStream oos = new ObjectOutputStream(new GZIPOutputStream (outputStream)))
    {
      // stream closed in the finally
      oos.writeObject(obj);
    }
    catch (Exception ex)
    {
      throw new SerializationException(ex);
    }
  }
  
  public static void serialize(Serializable obj, OutputStream outputStream) throws SerializationException
  {
    Preconditions.checkNotNull(outputStream);
    
    try(ObjectOutputStream out = new ObjectOutputStream(outputStream))
    {
      out.writeObject(obj); 
    }
    catch (Exception ex)
    {
      throw new SerializationException(ex);
    }  
  }
  
  public static byte[] zipSerialize(Serializable obj) throws SerializationException
  {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    zipSerialize(obj, baos);

    return baos.toByteArray();
  }
  
  public static byte[] serialize(Serializable obj) throws SerializationException
  {
    ByteArrayOutputStream baos = new ByteArrayOutputStream();
    serialize(obj, baos);
    return baos.toByteArray();
  }
   
  public static Object zipDeserialize(InputStream inputStream) throws SerializationException
  {
    Preconditions.checkNotNull(inputStream);
    
    try(ObjectInputStream ois = new ObjectInputStream(new GZIPInputStream (inputStream)))
    {     
      return ois.readObject();
    }
    catch (Exception ex)
    {
      throw new SerializationException(ex);
    }
  }
   
  public static Object deserialize(InputStream inputStream) throws SerializationException
  {
    Preconditions.checkNotNull(inputStream);
    
    try(ObjectInputStream in = new ObjectInputStream(inputStream))
    {
      return in.readObject();
    }
    catch (Exception ex)
    {
      throw new SerializationException(ex);
    } 
  }
   
  public static Object deserialize(byte[] objectData) throws SerializationException 
  {
    Preconditions.checkNotNull(objectData);
    ByteArrayInputStream bais = new ByteArrayInputStream(objectData);
    return deserialize(bais);
  }
   
  public static Object zipDeserialize(byte[] objectData) throws SerializationException
  {
    Preconditions.checkNotNull(objectData);
    ByteArrayInputStream bais = new ByteArrayInputStream(objectData);
    return zipDeserialize(bais);
  }
   
  public static Object deserialize(byte[] objectData, int offset, int length) throws SerializationException
  {
    Preconditions.checkNotNull(objectData);    
    ByteArrayInputStream bais = new ByteArrayInputStream(objectData, offset, length);
    return deserialize(bais);
  }
   
  public static Object zipDeserialize(byte[] objectData, int offset, int length) throws SerializationException
  {
    Preconditions.checkNotNull(objectData);
    ByteArrayInputStream bais = new ByteArrayInputStream(objectData, offset, length);
    return zipDeserialize(bais);
  }
  
  public static String zipBase64Serialize(Serializable obj) throws SerializationException
  {
    return Base64.encodeBase64URLSafeString(zipSerialize(obj));
  }
  
  public static Object zipBase64Deserialize(String data) throws SerializationException
  {
    return zipDeserialize(Base64.decodeBase64(data.getBytes()));
  }
  
  public static String zipBase64DeserializeAsString(String data) throws SerializationException
  {
    return (String)zipDeserialize(Base64.decodeBase64(data.getBytes()));
  }
}
