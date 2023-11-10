package com.sft.core.queue;

import com.google.common.base.Strings;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;

public class QueuePackage
{
  private final QueueWorker workerRef;
  private final Map<String, ByteBuffer> byteBufferMap;
  private final Integer id;
  private String info;
  private final String threadName;
  private int input;
  private int output;
  
  public static QueuePackage createCopy(QueuePackage qp)
  {
    QueuePackage newqp = new QueuePackage(qp.getWorkerRef(), qp.getId(), qp.getThreadName());
    
    for (Map.Entry<String, ByteBuffer> entry : qp.getByteBufferMap().entrySet())
    {
      newqp.addBuffer(entry.getKey(), entry.getValue());
    }
    
    newqp.setInfo(qp.getInfo());
    newqp.setInput(qp.getInput());
    newqp.setOutput(qp.getOutput());
    
    return newqp;
  }
  
  protected QueuePackage(QueueWorker wr, Integer aid, String threadname)
  {
    byteBufferMap = new HashMap<>();
    id = aid;
    workerRef = wr;
    threadName = threadname;
  }
  
  protected QueuePackage(QueueWorker wr, Integer aid)
  {
    byteBufferMap = new HashMap<>();
    id = aid;
    workerRef = wr;
    threadName = Thread.currentThread().getName();
  }

  public void clear()
  {
    byteBufferMap.clear();
  }
  
  /**
   * @return the id
   */
  public Integer getId() {
    return id;
  }

  /**
   * @return the byteBufferMap
   */
  public Map<String, ByteBuffer> getByteBufferMap() {
    return byteBufferMap;
  }
  
  public synchronized boolean addBuffer(String name, ByteBuffer buff)
  {
    if(Strings.isNullOrEmpty(name))
      return false;
    
    if(buff == null)
      return false;
    
    byteBufferMap.put(name, buff);

    return byteBufferMap.containsKey(name);
  }

  /**
   * @return the info
   */
  public String getInfo() {
    return info;
  }

  /**
   * @param info the info to set
   */
  public void setInfo(String info) {
    this.info = info;
  }

  /**
   * @return the threadName
   */
  public String getThreadName() {
    return threadName;
  }

  /**
   * @return the workerRef
   */
  public QueueWorker getWorkerRef()
  {
    return workerRef;
  }

  /**
   * @return the input
   */
  public int getInput()
  {
    return input;
  }

  /**
   * @param input the input to set
   */
  public void setInput(int input)
  {
    this.input = input;
  }

  /**
   * @return the output
   */
  public int getOutput()
  {
    return output;
  }

  /**
   * @param output the output to set
   */
  public void setOutput(int output)
  {
    this.output = output;
  }
}
