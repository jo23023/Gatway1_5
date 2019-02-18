#!/bin/bash

for f in $(ls src/*.c); do
echo $f
clang -O2 -g -Wall -Wunused-variable -fmessage-length=0 -Iinclude -Isrc --analyze $f
done

clang -O2 -g -Wall -Wunused-variable -fmessage-length=0 -Iinclude -Isrc --analyze *.c
