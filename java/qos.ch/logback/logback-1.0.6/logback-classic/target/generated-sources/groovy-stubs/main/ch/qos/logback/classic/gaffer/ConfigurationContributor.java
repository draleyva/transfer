//
// Generated stub from file:/C:/java/logback-1.0.6/logback-classic/src/main/groovy/ch/qos/logback/classic/gaffer/ConfigurationContributor.groovy
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

/**
 * @author Ceki G&uuml;c&uuml;
 */
public interface ConfigurationContributor
{
    /**
     * The list of method mapping from the contributor into the configuration mechanism,
     * e.g. the ConfiguratorDelegate
     * 
     * <p>The key in the map is the method being contributed and the value is the name of
     * the method in the target class.
     *
     * @return
     */
    Map getMappings();
}
