#!/bin/bash

# Use Java 15. Application must be exported in Processing IDE:
#   File -> Export Application (uncheck Embed Java for Linux)
# If you haven't Java 15 you can install it:
#   sudo apt install openjdk-15-jre
JAVA_HOME="/usr/lib/jvm/java-1.15.0-openjdk-amd64"
PATH="$JAVA_HOME/bin:$PATH"
/home/petr/Workspace/tv-ambient-led-strip/Processing/Adalight/application.linux64/Adalight

# Run sketch via Processing and its Java
# ~/Programy/processing-2.2.1/processing-java --sketch=/home/petr/Workspace/tv-ambient-led-strip/Processing/Adalight --output=/tmp/adalight --force --run
