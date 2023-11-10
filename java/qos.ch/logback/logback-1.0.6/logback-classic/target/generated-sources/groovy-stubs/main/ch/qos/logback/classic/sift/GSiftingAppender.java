//
// Generated stub from file:/C:/java/logback-1.0.6/logback-classic/src/main/groovy/ch/qos/logback/classic/sift/GSiftingAppender.groovy
//

package ch.qos.logback.classic.sift;

import java.lang.*;
import java.io.*;
import java.net.*;
import java.util.*;
import groovy.lang.*;
import groovy.util.*;
import java.math.BigDecimal;
import java.math.BigInteger;
import ch.qos.logback.core.AppenderBase;
import ch.qos.logback.classic.spi.ILoggingEvent;
import ch.qos.logback.core.sift.AppenderTrackerImpl;
import ch.qos.logback.core.sift.Discriminator;
import ch.qos.logback.core.sift.AppenderTracker;
import ch.qos.logback.core.Appender;
import ch.qos.logback.classic.gaffer.ConfigurationContributor;
import ch.qos.logback.core.CoreConstants;
import ch.qos.logback.core.helpers.NOPAppender;

public class GSiftingAppender
    extends AppenderBase
    implements groovy.lang.GroovyObject, ConfigurationContributor
{
    protected AppenderTracker appenderTracker = null;

    private Discriminator discriminator = null;
    public Discriminator getDiscriminator() {
        throw new InternalError("Stubbed method");
    }
    public void setDiscriminator(Discriminator value) {
        throw new InternalError("Stubbed method");
    }

    private Closure builderClosure = null;
    public Closure getBuilderClosure() {
        throw new InternalError("Stubbed method");
    }
    public void setBuilderClosure(Closure value) {
        throw new InternalError("Stubbed method");
    }

    private int nopaWarningCount = 0;
    public int getNopaWarningCount() {
        throw new InternalError("Stubbed method");
    }
    public void setNopaWarningCount(int value) {
        throw new InternalError("Stubbed method");
    }

    public Map getMappings() {
        throw new InternalError("Stubbed method");
    }

    public void start() {
        throw new InternalError("Stubbed method");
    }

    public void stop() {
        throw new InternalError("Stubbed method");
    }

    protected long getTimestamp(ILoggingEvent event) {
        throw new InternalError("Stubbed method");
    }

    public Appender buildAppender(java.lang.String value) {
        throw new InternalError("Stubbed method");
    }

    public void append(java.lang.Object object) {
        throw new InternalError("Stubbed method");
    }

    public NOPAppender buildNOPAppender(java.lang.String discriminatingValue) {
        throw new InternalError("Stubbed method");
    }

    public void build() {
        throw new InternalError("Stubbed method");
    }

    public void sift(Closure clo) {
        throw new InternalError("Stubbed method");
    }

    public AppenderTracker getAppenderTracker() {
        throw new InternalError("Stubbed method");
    }

    public java.lang.String getDiscriminatorKey() {
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
