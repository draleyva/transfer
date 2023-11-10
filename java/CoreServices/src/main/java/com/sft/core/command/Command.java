package com.sft.core.command;

public abstract class Command 
{
  protected long responseCode = 0;
  
  public long getResponseCode()
  {
    return responseCode;
  }

  public void setResponseCode(long responseCode)
  {
    this.responseCode = responseCode;
  }
  
  public abstract boolean isCommandResponse(byte [] commandbuffer, int commandlength);
  public abstract boolean isCommandRequirement(byte [] commandbuffer, int commandlength);
}
