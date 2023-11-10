package com.sft.core.queue;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class QueueFactory 
{
  ExecutorService threadExecutor;
  
  protected QueueFactory()
  {
    threadExecutor = Executors.newCachedThreadPool();
  }
  
  public static QueueFactory newQueueFactory()
  {
    return new QueueFactory();
  }
  
  public void startWorker(QueueWorker qw)
  {
    qw.start();
    threadExecutor.execute(qw);
  }
  
  public void stopWorker(QueueWorker qw)
  {
    qw.stop();
  }
}
