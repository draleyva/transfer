package com.sft.core.exception;

/**
 *
 * @author David Leyva
 */
import java.util.ArrayList;
import java.util.List;

public class EnrichableException extends RuntimeException
{
  private static final String SEPARATOR = "->";
          
  public static final long serialVersionUID = -1;

  protected final transient List<InfoItem> infoItems = new ArrayList<>();

  protected class InfoItem
  {
    protected String errorContext = null;
    protected String errorCode  = null;
    protected String errorText  = null;
    
    public InfoItem(String contextCode, String errorCode, String errorText)
    {
      this.errorContext = contextCode;
      this.errorCode   = errorCode;
      this.errorText   = errorText;
    }
  }


    public EnrichableException(String errorContext, String errorCode,
                               String errorMessage){

        addInfo(errorContext, errorCode, errorMessage);
    }

    public EnrichableException(String errorContext, String errorCode,
                               String errorMessage, Throwable cause){
        super(cause);
        addInfo(errorContext, errorCode, errorMessage);
    }

    public EnrichableException addInfo(
        String errorContext, String errorCode, String errorText){

        this.infoItems.add(
            new InfoItem(errorContext, errorCode, errorText));
        return this;
    }

    public String getCode(){
        StringBuilder builder = new StringBuilder();

        for(int i = this.infoItems.size()-1 ; i >=0; i--){
            InfoItem info =
                this.infoItems.get(i);
            builder.append('[');
            builder.append(info.errorContext);
            builder.append(':');
            builder.append(info.errorCode);
            builder.append(']');
        }

        return builder.toString();
    }

    @Override
    public String toString()
    {
        StringBuilder builder = new StringBuilder();

        builder.append(getCode());
        builder.append(SEPARATOR);


        //append additional context information.
        for(int i = this.infoItems.size()-1 ; i >=0; i--){
            InfoItem info =
                this.infoItems.get(i);
            builder.append('[');
            builder.append(info.errorContext);
            builder.append(':');
            builder.append(info.errorCode);
            builder.append(']');
            builder.append(info.errorText);
            if(i>0) builder.append(SEPARATOR);
        }

        //append root causes and text from this exception first.
        if(getMessage() != null) 
        {
          builder.append(SEPARATOR);
          if(getCause() == null || !getMessage().equals(getCause().toString()) )
          {
            builder.append(getMessage());
          }
        }
        appendException(builder, getCause());

        return builder.toString();
    }

    private void appendException
    (
            StringBuilder builder, Throwable throwable){
        if(throwable == null) return;
        appendException(builder, throwable.getCause());
        builder.append(throwable.toString());
        builder.append(SEPARATOR);
    }
}
