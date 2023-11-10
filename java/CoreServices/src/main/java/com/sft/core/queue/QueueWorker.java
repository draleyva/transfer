package com.sft.core.queue;

import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public abstract class QueueWorker implements Runnable
{
  LinkedBlockingQueue<QueuePackage> queue;
  Semaphore semaphore;
  boolean work;
  AtomicInteger workerIdGenerator = new AtomicInteger(0);
  
  public QueueWorker()
  {
    queue = new LinkedBlockingQueue<>();
    semaphore = new Semaphore(0, false);
  }
  
  public abstract void processPackage(QueuePackage qp);
  
  public void processException(String function, Exception e)
  {
    
  }
  
  public void processIssue(String message)
  {
    
  }
  
  public void processAdvice(String message)
  {
    
  }
  
  @Override
  public void run()
  {
    while(work)
    {
      try
      {
        semaphore.acquire();
        QueuePackage qp = queue.poll();
        
        if(qp != null)
        {
          processPackage(qp);
        }
      }
      catch (Exception e)
      {
        processException("run", e);
      }
    }

    processAdvice("Worker has been stopped");
    
    boolean result;
    int permits = semaphore.availablePermits();
    
    for(int i = 0; i < permits && !queue.isEmpty(); i++)
    {
      try
      {
        result = semaphore.tryAcquire(100, TimeUnit.MILLISECONDS);
        
        QueuePackage qp = queue.poll();
        
        if(qp != null)
        {
          if(!result)
          {
            processIssue("Acquire failed but package is present");
          }
          processPackage(qp);
        }
      }
      catch (Exception e)
      {
        processException("run", e);
      }
    }
  }
  
  protected void start()
  {
    work = true;
  }
  
  protected void stop()
  {
    semaphore.release(1);
    work = false;
  }
  
  public boolean push(QueuePackage qp)
  {
    try
    {
      if(work)
      {
        queue.put(qp);
        semaphore.release(1);
      
        return true;
      }
    }
    catch (Exception e)
    {
      processException("push", e);
    }
    
    return false;
  }
  
  public synchronized QueuePackage createPackage()
  {
    return new QueuePackage(this, workerIdGenerator.getAndIncrement());
  }
}
