import org.gradle.nativeplatform.platform.internal.DefaultNativePlatform

var SFTLIBRARY = "";

if (DefaultNativePlatform.getCurrentOperatingSystem().isWindows()) {
    SFTLIBRARY = "C:/SFTlibrary";
}
else {
    SFTLIBRARY = "/home/sanms/SFTlibrary";
}

tasks.jar {
    destinationDirectory.set(File("$projectDir/dist"));
}

tasks.clean {
    var file = File("$projectDir/dist")
    if(file.exists())
    {
        delete(fileTree("$projectDir/dist"))
    }
}

plugins {
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
    implementation(files("../LogServices/dist/LogServices.jar"))
    
    implementation("com.google.code.gson:gson:2.9.1")
    implementation("com.google.guava:guava:27.1-android")
    implementation("commons-codec:commons-codec:1.13")
    implementation("org.apache.commons:commons-configuration2:2.6")
    implementation("org.apache.commons:commons-pool2:2.8.0")
    implementation("org.freemarker:freemarker:2.3.30")
    implementation("org.slf4j:slf4j-api:1.7.21")
    
    logback(
        "core",
        "classic",
        "access"
    ).forEach { implementation(files(it)) }    
}

fun logback(
    vararg modules: String
): List<String> = modules.map { module ->
    SFTLIBRARY+"/qos.ch/logback/custom-logback-master/logback-$module-1.3.0-SNAPSHPOT.jar"
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

