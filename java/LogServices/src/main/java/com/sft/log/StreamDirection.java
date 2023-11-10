package com.sft.log;

/**
 *
 * @author David Ricardo
 */
public enum StreamDirection
{
  SRQ("SRQ", "Request"),
  SRS("SRS", "Response"),
  SNR("SNR", "No response"),
  NRQ("NRQ", "Request error"),
  SRD("SRD", "Redirection"),
  SRR("SRR", "Response redirection"),
  SND("SND", "Redirection error"),
  ARQ("ARQ", "Authorization request"),
  ARS("ARS", "Authorization response"),
  ANR("ANR", "Authorization no response"),
  DRQ("DRQ", "Request from Service"),
  DRS("DRS", "Response to Service");
  
  final String type;
  final String description;
  
  StreamDirection(String t, String d)
  {
    this.type = t;
    this.description = d;
  }
 
  public String getType()
  {
    return type;
  }
  
  public String getDescription()
  {
    return description;
  }
  
  public static StreamDirection fromString(String st)
  {
    if (st != null) {
      for (StreamDirection b : StreamDirection.values())
      {
        if (st.equalsIgnoreCase(b.type))
        {
          return b;
        }
      }
    }
    return null;
  }
}
