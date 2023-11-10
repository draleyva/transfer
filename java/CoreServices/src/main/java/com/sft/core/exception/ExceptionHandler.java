/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package com.sft.core.exception;

/**
 *
 * @author David Leyva
 */
public interface ExceptionHandler{

   public void handle(String contextCode, String errorCode,
                      String errorText, Throwable t);

   public void raise(String contextCode, String errorCode,
                     String errorText);
   
}
