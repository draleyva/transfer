package com.sft.core.validation;

import com.sft.core.converter.LongConvert;
import java.util.zip.CRC32;
import java.util.zip.Checksum;

public class MacRecord 
{
  protected MacRecord()
  {
  }
  
  public static String buildMAC(String record)
  {
    byte [] bytes = record.getBytes();
    byte [] checkbytes = null;
    long checksumvalue = 0;
    long checkvalue = 0;
    Checksum checksum = new CRC32();
    
    /*
    * To compute the CRC32 checksum for byte array, use
    *
    * void update(bytes[] b, int start, int length)
    * method of CRC32 class.
    */ 
    checksum.update(bytes,0,bytes.length);
    checksumvalue = checksum.getValue();
    
    checkbytes = Long.toString(checkvalue).getBytes();
    
    checksum.update(checkbytes,0,checkbytes.length);
    checkvalue = checksum.getValue();
    /*
    * Get the generated checksum using
    * getValue method of CRC32 class.
    */
    
    return LongConvert.longLong2String(checksumvalue, checkvalue);
  }
  
  public static boolean validateMAC(String record, String mac)
  {
    return buildMAC(record).equals(mac);
  }
}
