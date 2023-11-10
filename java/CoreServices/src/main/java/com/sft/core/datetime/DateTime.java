package com.sft.core.datetime;

import com.google.common.base.Strings;
import com.sft.core.exception.CoreException;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.Map;
import java.util.TimeZone;
import java.util.concurrent.ConcurrentHashMap;
import javax.xml.datatype.DatatypeConfigurationException;
import javax.xml.datatype.DatatypeFactory;
import javax.xml.datatype.XMLGregorianCalendar;

public class DateTime 
{
	public static final TimeZone UTCTZ = TimeZone.getTimeZone("UTC");
	
  public static final String DATETIME_MMDDHHMMSS = "MMddHHmmss";
  public static final String DATETIME_YYYYMMDDHHMMSS = "yyyyMMddHHmmss";
  public static final String DATETIME_YYMMDDHHMM = "yyMMddHHmm";
  public static final String DATETIME_YYYYMMDD = "yyyyMMdd";
  public static final String DATETIME_YYMMDD = "yyMMdd";
  public static final String DATETIME_HHMMSS = "HHmmss";
  public static final String DATETIME_HHMMSSSSS = "HHmmssSSS";
  public static final String DATETIME_YYYY_MM_DD_HH_MM_SS = "yyyy-MM-dd HH:mm:ss";
  public static final String DATETIME_YYYY_MM_DD = "yyyy-MM-dd";
  public static final String DATETIME_HH_MM_SS = "HH:mm:ss";
  public static final String DATETIME_YYYYMMDDHHMMSSSSS = "yyyyMMddHHmmssSSS";
  
  protected DateTime()
  {
  }
  
  public static boolean isGreaterOrEqual(String dt, String ref)
  {
    return dt.compareTo(ref) >= 0;
  }
  
  public static boolean isLowerOrEqual(String dt, String ref)
  {
    return dt.compareTo(ref) <= 0;
  }
  
  public static boolean isLower(String dt, String ref)
  {
    return dt.compareTo(ref) < 0;
  }
  
  public static boolean isEqual(String dt, String ref)
  {
    return dt.compareTo(ref) == 0;
  }
  
  public static boolean isBetween(String dt, String startdt, String enddt)
  {
    if(Strings.isNullOrEmpty(dt) || Strings.isNullOrEmpty(startdt) || Strings.isNullOrEmpty(enddt))
      return false;
    
    return (startdt.compareTo(dt) <= 0 && enddt.compareTo(dt) >= 0);
  }
  
  public static String getDateTime(String format)
  {
    return new SimpleDateFormat(format).format(new Date());
  }
  
  public static String getDateTime()
  {
    return getDateTime(DATETIME_YYYYMMDDHHMMSS);
  }
  
  public static String getDateTimeMS()
  {
    return getDateTime(DATETIME_YYYYMMDDHHMMSSSSS);
  }
  
  public static String getShortDateTime()
  {
    return getDateTime(DATETIME_MMDDHHMMSS);
  }
  
  public static String getDateTime(XMLGregorianCalendar gregoriancalendar, String format) 
  {
    Date date = gregoriancalendar.toGregorianCalendar().getTime();
    return getDateTime(date, format);
  }
  
  public static String getDateTime(Date date, String format) 
  {
    DateFormat dateFormat = new SimpleDateFormat(format);
    return dateFormat.format(date);
  }
  
  public static String getDateTime(Date date) 
  {
    DateFormat dateFormat = new SimpleDateFormat(DATETIME_YYYYMMDDHHMMSS);
    return dateFormat.format(date);
  }
  
  public static Date getDate(String date) throws ParseException
  {
    DateFormat dateFormat = new SimpleDateFormat(DATETIME_YYYYMMDD);
    return dateFormat.parse(date);
  }

  public static final XMLGregorianCalendar getXMLGregorianCalendarNow() throws CoreException
  {
    try
    {
      GregorianCalendar gregorianCalendar = new GregorianCalendar();
      DatatypeFactory datatypeFactory = DatatypeFactory.newInstance();
      return datatypeFactory.newXMLGregorianCalendar(gregorianCalendar);
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
 
  public static final XMLGregorianCalendar getXMLGregorianCalendarNowUTC() throws CoreException 
  {
    try
    {
      return DatatypeFactory.newInstance().newXMLGregorianCalendar((GregorianCalendar)GregorianCalendar.getInstance(UTCTZ));
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
  
  public static XMLGregorianCalendar getXMLGregorianCalendar(String format, String sdatetime) throws ParseException, DatatypeConfigurationException
  {
    GregorianCalendar gCalendar = new GregorianCalendar();
    SimpleDateFormat dateformat = new SimpleDateFormat(format);
    Date datetime;
    
    datetime = dateformat.parse(sdatetime);

    gCalendar.setTime(datetime);
    XMLGregorianCalendar xmlCalendar = null;
    
    xmlCalendar = DatatypeFactory.newInstance().newXMLGregorianCalendar(gCalendar);
    
    return xmlCalendar;
  }

  public static XMLGregorianCalendar getXMLGregorianCalendar(SimpleDateFormat dateformat, TimeZone tz, String sdatetime) throws ParseException, DatatypeConfigurationException
  {
    GregorianCalendar gCalendar = (GregorianCalendar)GregorianCalendar.getInstance(tz);
    Date datetime;
    
    datetime = dateformat.parse(sdatetime.replace("T", " "));

    gCalendar.setTime(datetime);
    XMLGregorianCalendar xmlCalendar;
    
    xmlCalendar = DatatypeFactory.newInstance().newXMLGregorianCalendar(gCalendar);
    
    return xmlCalendar;
  }
  
  static final Map<String, SimpleDateFormat> SIMPLEDATEFORMATMAP = new ConcurrentHashMap<>();
  
  public static String changeFormat(String newFormat, String oldFormat, String sdatetime) throws ParseException
  {
    return changeFormat(newFormat, oldFormat, sdatetime, null);
  }
  
  public static class DateFormatter
  {
    protected SimpleDateFormat oldFormat;
    protected SimpleDateFormat newFormat;
    
    protected DateFormatter()
    {
    }
    
    public static DateFormatter createDateFormatter(String oldformat, String newformat)
    {
      DateFormatter df = new DateFormatter();
      
      df.oldFormat = new SimpleDateFormat(oldformat);
      df.newFormat = new SimpleDateFormat(newformat);
      
      return df;
    }
  }
  
  public static String changeFormat(DateFormatter df, String sdatetime, String defaultvalue) throws ParseException
  {
    Date datetime;
    
    try
    {
      datetime = df.oldFormat.parse(sdatetime);
    }
    catch(ParseException e)
    {
      if(defaultvalue != null)
      {
        return defaultvalue;
      }
      else
      {
        throw e;
      }
    }
    
    return df.newFormat.format(datetime);
  }
  
  public static String changeFormat(String newFormat, String oldFormat, String sdatetime, String defaultvalue) throws ParseException
  {
    SimpleDateFormat dateOldFormat;
    SimpleDateFormat dateNewFormat;
    
    if(!SIMPLEDATEFORMATMAP.containsKey(oldFormat))
    {
      dateOldFormat = SIMPLEDATEFORMATMAP.put(oldFormat, new SimpleDateFormat(oldFormat));
    }
    else
    {
      dateOldFormat = SIMPLEDATEFORMATMAP.get(oldFormat);
    }
    
    if(!SIMPLEDATEFORMATMAP.containsKey(newFormat))
    {
      dateNewFormat = SIMPLEDATEFORMATMAP.put(newFormat, new SimpleDateFormat(newFormat));
    }
    else
    {
      dateNewFormat = SIMPLEDATEFORMATMAP.get(newFormat);
    }

    Date datetime;
    
    try
    {
      datetime = dateOldFormat.parse(sdatetime);
    }
    catch(ParseException e)
    {
      if(defaultvalue != null)
      {
        return defaultvalue;
      }
      else
      {
        throw e;
      }
    }
    
    return dateNewFormat.format(datetime);
  }
  
  public static String getDate()
  {
    return new SimpleDateFormat(DATETIME_YYYYMMDD).format(new Date());
  }

  public static String getDate(Date date)
  {
    return new SimpleDateFormat(DATETIME_YYYYMMDD).format(date);
  }
  
  public static String getDateShort()
  {
    return new SimpleDateFormat(DATETIME_YYMMDD).format(new Date());
  }
  
  public static String getDateShort(Date date)
  {
    return new SimpleDateFormat(DATETIME_YYMMDD).format(date);
  }
  
  public static String getTime()
  {
    return new SimpleDateFormat(DATETIME_HHMMSS).format(new Date());
  }

  public static String getTime(Date date)
  {
    return new SimpleDateFormat(DATETIME_HHMMSS).format(date);
  }
  
  public static String getTimeMS()
  {
    return new SimpleDateFormat(DATETIME_HHMMSSSSS).format(new Date());
  }
  
  public static String getTimeMS(Date date)
  {
    return new SimpleDateFormat(DATETIME_HHMMSSSSS).format(date);
  }
  
  public static String getDateTimeShort()
  {
    return new SimpleDateFormat(DATETIME_YYMMDDHHMM).format(new Date());
  }
  
	public static long toLocalTime(long time, TimeZone to)
	{
		return convertTime(time, UTCTZ, to);
  }

  public static long toUTC(long time, TimeZone from)
	{
		return convertTime(time, from, UTCTZ);
  }

  public static long convertTime(long time, TimeZone from, TimeZone to)
	{
		return time + getTimeZoneOffset(time, from, to);
  }

  private static long getTimeZoneOffset(long time, TimeZone from, TimeZone to)
	{
		int fromOffset = from.getOffset(time);
    int toOffset = to.getOffset(time);
    int diff = 0;

    if (fromOffset >= 0)
		{
      if (toOffset > 0)
			{
				toOffset = -1*toOffset;
      }
			else
			{
				toOffset = Math.abs(toOffset);
      }
			diff = (fromOffset+toOffset)*-1;
    }
		else
		{
			if (toOffset <= 0)
			{
				toOffset = -1*Math.abs(toOffset);
      }
      diff = (Math.abs(fromOffset)+toOffset);
    }
    return diff;
  }
  
  public static long diffTime(String start, String end)
  {
    SimpleDateFormat format = new SimpleDateFormat(DATETIME_YYYYMMDDHHMMSS);
    Date d1;
    Date d2;
    try
    {
      d1 = format.parse(start);
      d2 = format.parse(end);
    }
    catch (Exception e)
    {
      return -1;
    }
    
    return d2.getTime() - d1.getTime();
  }
}
