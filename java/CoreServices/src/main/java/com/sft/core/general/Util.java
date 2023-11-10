package com.sft.core.general;

import ch.qos.logback.classic.Logger;
import ch.qos.logback.core.ContextBase;
import ch.qos.logback.core.rolling.RolloverFailure;
import ch.qos.logback.core.rolling.helper.RenameUtil;
import ch.qos.logback.core.util.FileUtil;
import com.google.common.base.Strings;
import com.sft.core.exception.CoreException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.RandomAccessFile;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URL;
import java.nio.ByteBuffer;
import java.nio.charset.Charset;
import java.text.DecimalFormat;
import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Util extends ContextBase
{ 
  protected static final Logger LOGGER = (Logger)org.slf4j.LoggerFactory.getLogger(Util.class);
  
  public static final int MB = 1024*1024;
  public static final int TIME_S2MS = 1000;
  protected static DefaultHashMap<String, String> setMethod = new DefaultHashMap<>();
  protected static DefaultHashMap<String, String> getMethod = new DefaultHashMap<>();
  static final RenameUtil RENAMEUTIL = new RenameUtil();
  static final Util UTIL = new Util();
  
  static
  {
    RENAMEUTIL.setContext(UTIL);
  }
  
  public static int second2ms(int seconds)
  {
    return seconds*TIME_S2MS;
  }
  
  public static int ms2seconds(int ms)
  {
    return ms/TIME_S2MS;
  }
  
  public static int ms2seconds(long ms)
  {
    return (int)(ms/TIME_S2MS);
  }
  
  public static void writeByteBuffer2File(String file, ByteBuffer bb, int offset, int len) throws IOException
  {
    try(FileOutputStream stream = new FileOutputStream(file))
    {
      stream.write(bb.array(), offset, len);
    }
  }
	
  public static String normalizeFilePath(String file)
  {
    if(Strings.isNullOrEmpty(file))
      return file;
    
    try
    {
      file = Normalizer.normalize(file, Form.NFKC).replace("//", "/");
    
      return file.replace("\\", "/");
    }
    catch(Exception e)
    {
      return null;
    }
  }

  public static boolean isPrintableAscii(byte value)
  {
    return value >= 32 ;
  }

  public static boolean isPrintable(byte[] buffer, int offset, int bufferSize)
  {
    for( int index = 0; index < bufferSize; ++index)
    {
      byte current = buffer[offset+index];
      if(!isPrintableAscii(current))
      {
        return false;
      }
    }

    return true;
  }
  
  public static String normalizeUrl(String urlstring) throws CoreException
  {
    if(Strings.isNullOrEmpty(urlstring))
      return urlstring;
    
    try
    {
      URL url = new URI(urlstring).normalize().toURL();
      return url.toString();
    }
    catch(Exception e)
    {
      throw new CoreException(e);
    }
  }
	
  public static String removeExtension(String s)
  {
    String separator = System.getProperty("file.separator");
    String filename;

    // Remove the path upto the filename.
    int lastSeparatorIndex = s.lastIndexOf(separator);
    if (lastSeparatorIndex == -1) {
      filename = s;
    } else {
      filename = s.substring(lastSeparatorIndex + 1);
    }

    // Remove the extension.
    int extensionIndex = filename.lastIndexOf('.');
    if (extensionIndex == -1)
      return filename;

    return filename.substring(0, extensionIndex);
  }
	
  static final String ZERO_S = "0";
  static final char ZERO_C = '0';
  
  public static String padNumeric(String string, int minLength) 
  {
    String value;
 
    if(Strings.isNullOrEmpty(string))
    {
      value = ZERO_S;
    }
    else
    {
      value = new BigDecimal(string).toBigInteger().toString();
      value = value.replace(".", "");
      value = value.replace(",", "");
      value = value.replace(" ", "");
    }
    
    if(value.length() > minLength)
      value = value.substring(0, minLength);
    
    return Strings.padStart(value, minLength, ZERO_C);
  }
  
  public static String padNumeric(long val, int minLength) 
  {
    return Util.padNumeric(String.valueOf(val), minLength);
  }

  public static String padNumeric(double val, int precision, int minLength) 
  {
    return Util.padNumeric(String.valueOf(val*Math.pow(10, precision)), minLength);
  }
  
  public static String padNumeric(int val, int minLength) 
  {
    return Util.padNumeric(String.valueOf(val), minLength);
  }
  
  public static String readableFileSize(long size, int pow, String symbol)
  {
    if(size <= 0)
      return "0 "+symbol;
    if(size < 1024)
      size = 1024;
    return new DecimalFormat("#,##0.#").format(Math.round(size/Math.pow(1024.0f, pow))) + " "+symbol;
  }
  
  public static int string2size(String size)
  {
    int factor = 1;
    int number = 0;
    String stringsize;
    stringsize = size.toLowerCase();
    
    if(stringsize.contains("kb") || stringsize.contains("k"))
    {
      stringsize = stringsize.replace("kb", "");
      stringsize = stringsize.replace("k", "");
      factor = 1024;
    }
    if(stringsize.contains("mb") || stringsize.contains("m"))
    {
      stringsize = stringsize.replace("mb", "");
      stringsize = stringsize.replace("m", "");
      factor = 1024*1024;
    }
    if(stringsize.contains("b"))
    {
      factor = 1;
    }
    
    try
    {
      number = Integer.parseInt(stringsize);
      number = number*factor;
    }
    catch(Exception e)
    {
      number = -1;
    }
    
    return number;
  }
  
	public static boolean containsAny(String str, String searchChars)
	{
		if (searchChars == null)
		{
			return false;
    }
    return containsAny(str, searchChars.toCharArray());
  }
	
	public static boolean containsAny(String str, char[] searchChars)
	{
		if (str == null || str.length() == 0 || searchChars == null || searchChars.length == 0)
		{
			return false;
    }
    for (int i = 0; i < str.length(); i++)
		{
			char ch = str.charAt(i);
      for (int j = 0; j < searchChars.length; j++)
			{
				if (searchChars[j] == ch)
				{
					return true;
        }
      }
    }
    return false;
  }
	
  public static boolean parseBoolean(String value)
  {
    if(Strings.isNullOrEmpty(value))
      return false;
      
    return value.equals("1") || value.equalsIgnoreCase("yes") || value.equalsIgnoreCase("true") || value.equalsIgnoreCase("si") || value.equalsIgnoreCase("y");
  }
  
  public static String boolean2string(Boolean b)
  {
    return b?"1":"0";
  }
  
  public static URL path2URL(String filepath) throws MalformedURLException
  {
    File file = new File(filepath);
    
    return file.toURI().toURL();
  }
  
  public static void createMissingTargetDirsIfNecessary(File toFile) throws CoreException
  {
    if (isParentDirectoryCreationRequired(toFile)) {
      boolean result = createMissingParentDirectories(toFile);
      if (!result) 
      {
        throw new CoreException("Failed to create parent directories for <"+ toFile.getAbsolutePath() + ">");
      }
    }
  }
  
  public static boolean isParentDirectoryCreationRequired(File file)
  {
    File parent = file.getParentFile();
    return parent != null && !parent.exists();
  }

  public static boolean createMissingParentDirectories(File file)
  {    
    return FileUtil.createMissingParentDirectories(file);
  }
  
  public static void rename(String srcFile, String targetFile) throws CoreException
  {
    try
    {
      RENAMEUTIL.rename(srcFile, targetFile);
    }
    catch(RolloverFailure e)
    {
      throw new CoreException(e);
    }
  }
  
  public static void rename(File srcFile, File targetFile) throws CoreException
  {
    rename(srcFile.getAbsolutePath(), targetFile.getAbsolutePath());
  }
  
  public static boolean isClass(String className)
  {
    boolean exist = true;
    try 
    {
      Class.forName(className);
    } 
    catch (ClassNotFoundException e) 
    {
      exist = false;
    }
    return exist;
  }
  
  public static int indexOf(Pattern pattern, String s) 
  {
    Matcher matcher = pattern.matcher(s);
    return matcher.find() ? matcher.start() : -1;
  }
  
  public static String readFileContent(String filename, Charset charset) throws IOException 
  {
    try(RandomAccessFile raf = new RandomAccessFile(filename, "r"))
    {
      byte[] buffer = new byte[(int)raf.length()];
      raf.readFully(buffer);
      return new String(buffer, charset);
    }
  }
  
  public static String bigInteger2String(BigInteger bi)
  {
    if(bi == null)
      return null;
    
    return bi.toString();
  }
  
  public static String long2String(Long bi)
  {
    if(bi == null)
      return null;
    
    return bi.toString();
  }
  
  public static String double2String(Double bi)
  {
    if(bi == null)
      return null;
    
    return bi.toString();
  }
  
  public static double string2double(String s, int offset, int length)
  {
    try
    {
      String ls = s.substring(offset, length);
      return Double.parseDouble(ls);
    }
    catch(Exception e)
    {
    }
    
    return 0;
  }
  
  public static long string2long(String s, int offset, int length)
  {
    try
    {
      s = s.substring(offset, length);
      return Long.parseLong(s);
    }
    catch(Exception e)
    {
      // unable to manage
    }
    
    return 0;
  }
  
  public static String substring(String s, int offset, int length)
  {
    try
    {
      return s.substring(offset, offset+length);
    }
    catch(Exception e)
    {
      return "";
    }
  }
  
  public static String lastString(String s, int length)
  {
    int sl = s.length();
    if(length >= sl)
      return s;
    
    int no = s.length() - length;
    
    try
    {
      return s.substring(no);
    }
    catch(Exception e)
    {
      return "";
    }
  }
  
  public static int string2int(String s, int offset, int length)
  {
    return string2int(s.substring(offset, length));
  }
  
  public static int string2int(String s)
  {
    try
    {
      return Integer.parseInt(s);
    }
    catch(Exception e)
    {
      // unable to manage
    }
    
    return 0;
  }
  
  public static Double normalizeDouble(Double d)
  {
    if(d == null)
      return 0.0;
    
    return d;
  }
  
  public static String wildcardToRegex(String wildcard)
  {
    StringBuilder s = new StringBuilder(wildcard.length());
    s.append('^');
    for (int i = 0, is = wildcard.length(); i < is; i++)
    {
      char c = wildcard.charAt(i);
      switch(c)
      {
        case '*':
          s.append(".*");
        break;
        case '?':
          s.append(".");
        break;
        case '^': // escape character in cmd.exe
          s.append("\\");
        break;
        // escape special regexp-characters
        case '(': case ')': case '[': case ']': case '$':
        case '.': case '{': case '}': case '|':
        case '\\':
          s.append("\\");
          s.append(c);
        break;
        default:
          s.append(c);
        break;
      }
    }
    s.append('$');
    return(s.toString());
  }
  
  static final Map<String, Pattern> wildcardMatchPatternMap = new ConcurrentHashMap<>();
  
  public static boolean wildcardMatch(String s, String match)
  {
    Pattern r;
    
    if(wildcardMatchPatternMap.containsKey(match))
      r = wildcardMatchPatternMap.get(match);
    else
    {
      r = Pattern.compile(wildcardToRegex(match));
      wildcardMatchPatternMap.put(match, r);
    }
    
    Matcher m = r.matcher(s);
    
    return m.find();
  }
  
  public static String capitalizeFirstLetter(String s)
  {
    if (Strings.isNullOrEmpty(s)) return s;
    return s.substring(0, 1).toUpperCase() + s.substring(1);
  }
  
  public static String getSetMethod(String variablename)
  {
    if(setMethod.containsKey(variablename))
      return setMethod.get(variablename);
    
    String ssm = "set"+Util.capitalizeFirstLetter(variablename);
    
    setMethod.put(variablename, ssm);
    
    return ssm;
  }
  
  public static String getGetMethod(String variablename)
  {
    if(getMethod.containsKey(variablename))
      return getMethod.get(variablename);
    
    String gsm = "get"+Util.capitalizeFirstLetter(variablename);
    
    getMethod.put(variablename, gsm);
    
    return gsm;
  }
  
  public static void copy(InputStream source, OutputStream target) throws IOException
  {
    byte[] buf = new byte[8192];
    int length;
    while ((length = source.read(buf)) != -1)
      target.write(buf, 0, length);
  }
}
