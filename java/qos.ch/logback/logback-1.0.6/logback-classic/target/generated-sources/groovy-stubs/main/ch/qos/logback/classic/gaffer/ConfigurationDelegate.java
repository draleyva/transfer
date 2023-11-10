//
// Generated stub from file:/C:/java/logback-1.0.6/logback-classic/src/main/groovy/ch/qos/logback/classic/gaffer/ConfigurationDelegate.groovy
//

package ch.qos.logback.classic.gaffer;

import java.lang.*;
import java.io.*;
import java.net.*;
import java.util.*;
import groovy.lang.*;
import groovy.util.*;
import java.math.BigDecimal;
import java.math.BigInteger;
import ch.qos.logback.classic.Level;
import ch.qos.logback.classic.Logger;
import ch.qos.logback.classic.LoggerContext;
import ch.qos.logback.classic.turbo.ReconfigureOnChangeFilter;
import ch.qos.logback.classic.turbo.TurboFilter;
import ch.qos.logback.core.Appender;
import ch.qos.logback.core.CoreConstants;
import ch.qos.logback.core.spi.ContextAwareBase;
import ch.qos.logback.core.status.StatusListener;
import ch.qos.logback.core.util.CachingDateFormatter;
import ch.qos.logback.core.util.Duration;
import ch.qos.logback.core.spi.LifeCycle;
import ch.qos.logback.core.spi.ContextAware;

/**
 * @author Ceki G&uuml;c&uuml;
 */
public class ConfigurationDelegate
    extends ContextAwareBase
    implements groovy.lang.GroovyObject
{
    private List appenderList = null;
    public List getAppenderList() {
        throw new InternalError("Stubbed method");
    }
    public void setAppenderList(List value) {
        throw new InternalError("Stubbed method");
    }

    public java.lang.Object getDeclaredOrigin() {
        throw new InternalError("Stubbed method");
    }

    public void scan(java.lang.String scanPeriodStr) {
        throw new InternalError("Stubbed method");
    }

    public void statusListener(Class listenerClass) {
        throw new InternalError("Stubbed method");
    }

    public void conversionRule(java.lang.String conversionWord, Class converterClass) {
        throw new InternalError("Stubbed method");
    }

    public void root(Level level, List appenderNames) {
        throw new InternalError("Stubbed method");
    }

    public void logger(java.lang.String name, Level level, List appenderNames, Boolean additivity) {
        throw new InternalError("Stubbed method");
    }

    public void appender(java.lang.String name, Class clazz, Closure closure) {
        throw new InternalError("Stubbed method");
    }

    private void copyContributions(AppenderDelegate appenderDelegate, Appender appender) {
        throw new InternalError("Stubbed method");
    }

    public void turboFilter(Class clazz, Closure closure) {
        throw new InternalError("Stubbed method");
    }

    public java.lang.String timestamp(java.lang.String datePattern, long timeReference) {
        throw new InternalError("Stubbed method");
    }

    public groovy.lang.MetaClass getMetaClass() {
        throw new InternalError("Stubbed method");
    }

    public void setMetaClass(groovy.lang.MetaClass metaClass) {
        throw new InternalError("Stubbed method");
    }

    public java.lang.Object invokeMethod(java.lang.String name, java.lang.Object args) {
        throw new InternalError("Stubbed method");
    }

    public java.lang.Object getProperty(java.lang.String name) {
        throw new InternalError("Stubbed method");
    }

    public void setProperty(java.lang.String name, java.lang.Object value) {
        throw new InternalError("Stubbed method");
    }
}
