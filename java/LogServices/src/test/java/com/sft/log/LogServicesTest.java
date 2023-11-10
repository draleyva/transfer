package com.sft.log;

import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.function.Executable;

public class LogServicesTest
{  
  @Test
  public void checkingNullException()
  {
    assertThrows(IllegalArgumentException.class, new Executable() 
    {             
      @Override
      public void execute() throws Throwable 
      {
        String configuration = null;
        LogConfigurator.configure(configuration);
      }
    });
  }

  @Test 
  public void dummyReturnTrue()
  { 
    assertTrue(true, "someLibraryMethod should return 'true'");
  }
}
