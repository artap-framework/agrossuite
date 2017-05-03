#!/bin/sh

# compile example
mcs -out:test.exe org/*.cs main.cs

# run
./test.exe
