package com.sft.core.type;

import com.google.gson.JsonDeserializationContext;
import com.google.gson.JsonDeserializer;
import com.google.gson.JsonElement;
import com.google.gson.JsonParseException;
import java.lang.reflect.Type;

/**
 *
 * @author sanms
 */
public class VariableTypeDeserializer implements JsonDeserializer<VariableType>
{
  @Override
  public VariableType deserialize(JsonElement je, Type type, JsonDeserializationContext jdc) throws JsonParseException
  {
    if(je.isJsonNull())
      return null;
    
    if(je.isJsonObject())
    {
      return null;
    }
   
    String t = "";
    
    if(je.isJsonPrimitive())
      t = je.getAsString();
    
    return VariableType.fromString(t);
  }
  
}
