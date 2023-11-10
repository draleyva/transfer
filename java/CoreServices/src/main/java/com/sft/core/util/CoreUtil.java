package com.sft.core.util;

/**
 *
 * @author David Ricardo
 */
public class CoreUtil
{
   // Reference GWTUtil
  public static final String ENTITY_NAME = "ENTITY_NAME";
  public static final int AUDIT_TYPE_CREATE = 0;
  public static final int AUDIT_TYPE_READ = 1;
  public static final int AUDIT_TYPE_UPDATE = 2;
  public static final int AUDIT_TYPE_DELETE = 3;
  public static final String [] AUDIT_TYPE = new String [] {"CRE", "REA", "UPD", "DEL"};
  
  protected CoreUtil()
  {
  }
}
