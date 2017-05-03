#!/bin/sh

# compile package
javac org/agros/*.java

# compile example
javac main.java 
# run
java -Djava.library.path=. -cp . main
