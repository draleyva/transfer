package com.sft.core.command;

import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.sft.core.converter.ByteConvert;
import com.sft.core.converter.StringUtil;

public class ControlCommand extends Command
{
  public static final int COMMANDSIZE = 1024;
	// Indicadores de comando por defecto
  private String commandRequest = "COMMANDRQ";
  private String commandResponse = "COMMANDRS";
  static final int COMMAND_SIZE = 40;
  static final int ORDER_SIZE = 40;
  static final int RESPONSECODE_SIZE = 6;
  
  private String order = ""; 
  private String options = "";
  private String rawOptions = "";
  protected int optionsLength = 0;
  
	public ControlCommand()
	{
		
	}
	
	public ControlCommand(String request, String response)
	{
		Preconditions.checkNotNull(request, "Invalid request identifier");
		Preconditions.checkNotNull(response, "Invalid response identifier");

		commandRequest = request;
		commandResponse  = response;
	}
	
  public byte[] buildCommandRequirement(String order, String options)
  {
    String commandstring;
    
    if(!Strings.isNullOrEmpty(options))
    {
      optionsLength = options.length();
      commandstring = StringUtil.padEnd(getCommandRequest(), COMMAND_SIZE)+StringUtil.padEnd(order, ORDER_SIZE)+StringUtil.padEnd(options, optionsLength);
    }
    else
    {
      optionsLength = 0;
      commandstring = StringUtil.padEnd(getCommandRequest(), COMMAND_SIZE)+StringUtil.padEnd(order, ORDER_SIZE);
    }
    
    return commandstring.getBytes();
  }
  
  public byte[] buildCommandResponse(String order, long responsecode, String options)
  {
    String commandstring;
    
    if(options == null)
    {
      optionsLength = 0;
      commandstring = StringUtil.padEnd(getCommandResponse(), COMMAND_SIZE)+StringUtil.padEnd(order, ORDER_SIZE)+String.format("%0"+RESPONSECODE_SIZE+"d", responsecode);
    }
    else
    {
      optionsLength = options.length();
      commandstring = StringUtil.padEnd(getCommandResponse(), COMMAND_SIZE)+StringUtil.padEnd(order, ORDER_SIZE)+String.format("%0"+RESPONSECODE_SIZE+"d", responsecode)+StringUtil.padEnd(options, optionsLength);
    }
    
    return commandstring.getBytes();
  }
  
  public byte[] buildCommandResponse(long responsecode, String options)
  {
    return buildCommandResponse(order, responsecode, options);
  }
  
  public byte[] buildCommandResponse(long responsecode)
  {
    return buildCommandResponse(order, responsecode, null);
  }
  
	@Override
  public boolean isCommandResponse(byte [] commandbuffer, int commandlength)
  {
    String commandstring;
    String responsecodestring;
    
    commandstring = ByteConvert.bytes2String(commandbuffer, commandlength);
    
    try
    {
      if(!commandstring.substring(0, COMMAND_SIZE).trim().equals(commandResponse))
        return false;
        
      order = commandstring.substring(COMMAND_SIZE, ORDER_SIZE+COMMAND_SIZE).trim();
    
      if((COMMAND_SIZE+ORDER_SIZE+RESPONSECODE_SIZE) > commandlength)
        return false;
        
      responsecodestring = commandstring.substring(COMMAND_SIZE+ORDER_SIZE, COMMAND_SIZE+ORDER_SIZE+RESPONSECODE_SIZE).trim();
      responseCode = Integer.parseInt(responsecodestring);
              
      optionsLength = commandlength-(COMMAND_SIZE+ORDER_SIZE+RESPONSECODE_SIZE);
      
      if(optionsLength > 0)
      {
        rawOptions = commandstring.substring(COMMAND_SIZE+ORDER_SIZE+RESPONSECODE_SIZE, COMMAND_SIZE+ORDER_SIZE+RESPONSECODE_SIZE+optionsLength);
        options = rawOptions.trim();
      }
      else
      {
        options = "";
        rawOptions = "";
      }
    }
    catch(Exception e)
    {
      return false;
    }
    
    return true;
  }
          
	@Override
  public boolean isCommandRequirement(byte [] commandbuffer, int commandlength)
  {
    String commandstring;
    
    commandstring = ByteConvert.bytes2String(commandbuffer, commandlength);
    
    try
    {
      if(!commandstring.substring(0, COMMAND_SIZE).trim().equals(commandRequest))
        return false;
        
      order = commandstring.substring(COMMAND_SIZE, ORDER_SIZE+COMMAND_SIZE).trim();
    
      optionsLength = commandlength-(COMMAND_SIZE+ORDER_SIZE);
      
      if(optionsLength > 0)
        options = commandstring.substring(COMMAND_SIZE+ORDER_SIZE, COMMAND_SIZE+ORDER_SIZE+optionsLength).trim();
      else
        options = "";
    }
    catch(Exception e)
    {
      return false;
    }
    
    return true;
  }
  
  public String getOrder()
  {
    return order;
  }
  public String getOptions()
  {
    return options;
  }

  /**
   * @return the rawOptions
   */
  public String getRawOptions() {
    return rawOptions;
  }

	/**
	 * @return the commandRequest
	 */
	public String getCommandRequest() {
		return commandRequest;
	}

	/**
	 * @param commandRequest the commandRequest to set
	 */
	public void setCommandRequest(String commandRequest) {
		this.commandRequest = commandRequest;
	}

	/**
	 * @return the commandResponse
	 */
	public String getCommandResponse() {
		return commandResponse;
	}

	/**
	 * @param commandResponse the commandResponse to set
	 */
	public void setCommandResponse(String commandResponse) {
		this.commandResponse = commandResponse;
	}
}
