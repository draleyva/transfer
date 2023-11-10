import org.gradle.nativeplatform.platform.internal.DefaultNativePlatform

var SFTLIBRARY = "";
var SFTDEVELOPMENT = "";

if (DefaultNativePlatform.getCurrentOperatingSystem().isWindows()) {
    SFTLIBRARY = "C:/SFTlibrary";
    SFTDEVELOPMENT = "C:/SFTdevelopment";
}
else {
    SFTLIBRARY = System.getenv("HOME")+"/SFTlibrary";
    SFTDEVELOPMENT = System.getenv("HOME")+"/SFTdevelopment";
}

tasks.jar {
    destinationDirectory.set(File("$projectDir/dist"));
    // println(destinationDirectory.get());
}

tasks.clean {
    delete("$projectDir/dist");
}

plugins {
    // Apply the java-library plugin for API and implementation separation.
    `java-library`
}

java {
    toolchain {
        languageVersion.set(JavaLanguageVersion.of(8))
    }
}

repositories {
    mavenCentral()
}

dependencies {
    implementation("com.google.guava:guava:27.1-android")
    implementation("org.slf4j:slf4j-api:1.7.21");

    logback(
        "core",
        "classic",
        "access"
    ).forEach { implementation(files(it)) }

    testImplementation("org.junit.jupiter:junit-jupiter-api:5.3.1")
    testRuntimeOnly("org.junit.jupiter:junit-jupiter-engine:5.3.1")
}

testing {
    suites {
        // Configure the built-in test suite
        val test by getting(JvmTestSuite::class) {
            // Use JUnit4 test framework
            useJUnit("4.13.2")
        }
    }
}

fun logback(
    vararg modules: String
): List<String> = modules.map { module ->
    SFTLIBRARY+"/qos.ch/logback/custom-logback-master/logback-$module-1.3.0-SNAPSHPOT.jar"
}
