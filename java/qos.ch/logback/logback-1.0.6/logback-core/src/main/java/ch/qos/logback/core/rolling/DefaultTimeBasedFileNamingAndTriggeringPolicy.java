package ch.qos.logback.core.rolling;

import java.io.File;
import java.util.Date;

import ch.qos.logback.core.rolling.helper.TimeBasedArchiveRemover;

/**
 * 
 * @author Ceki G&uuml;lc&uuml;
 * 
 * @param <E>
 */
public class DefaultTimeBasedFileNamingAndTriggeringPolicy<E> extends
    TimeBasedFileNamingAndTriggeringPolicyBase<E> {

  @Override
  public void start() {
    super.start();
    archiveRemover = new TimeBasedArchiveRemover(tbrp.fileNamePattern, rc);
    archiveRemover.setContext(context);
    started = true;
  }

  public boolean isTriggeringEvent(File activeFile, final E event) {
    long time = getCurrentTime();
    if (time >= nextCheck) {
      Date dateOfElapsedPeriod = dateInCurrentPeriod;
      addInfo("Elapsed period: "+dateOfElapsedPeriod);
      elapsedPeriodsFileName = tbrp.fileNamePatternWCS
          .convert(dateOfElapsedPeriod);
      setDateInCurrentPeriod(time);
      computeNextCheck();
      return true;
    } else {
      return false;
    }
  }

  @Override
  public String toString() {
    return "c.q.l.core.rolling.DefaultTimeBasedFileNamingAndTriggeringPolicy";
  }
}
