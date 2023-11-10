package com.sft.core.general;

public abstract class LoopTask extends Thread
{
  private long lastTime = 0;
  private long trips = 0;
  private long sleepTime;
  boolean waitAndExecute = false;
  int loopCounter;
  
  public LoopTask(long sleep, int loop, boolean wait)
  {
    sleepTime = sleep;
    waitAndExecute = wait;
    loopCounter = loop;
  }
  
  @Override
  public void run()
  {
    long tick;
    int loop = 0;
    boolean trigger;
    
    while(!isInterrupted())
    {
      try
      {
        ++loop;
        if(loop%loopCounter == 0)
        {
          trigger = true;
          loop = 0;
        }
        else
          trigger = false;
        tick = System.currentTimeMillis();
        if(waitAndExecute)
          sleep( sleepTime );
        task(trigger);
        if(!waitAndExecute)
          sleep( sleepTime );
        tick = System.currentTimeMillis() - tick;
        
        lastTime = tick;
        ++trips;
      }
      catch(Exception e )
      {
        // unable to manage
      }
    }
  }
  
  public abstract void task(boolean trigger);
  
  /**
   * @return the lastTime
   */
  public long getLastTime() {
    return lastTime;
  }

  /**
   * @param lastTime the lastTime to set
   */
  public void setLastTime(long lastTime) {
    this.lastTime = lastTime;
  }

  /**
   * @return the trips
   */
  public long getTrips() {
    return trips;
  }

  /**
   * @param trips the trips to set
   */
  public void setTrips(long trips) {
    this.trips = trips;
  }

  /**
   * @return the sleepTime
   */
  public long getSleepTime() {
    return sleepTime;
  }

  /**
   * @param sleepTime the sleepTime to set
   */
  public void setSleepTime(long sleepTime) {
    this.sleepTime = sleepTime;
  }
}
