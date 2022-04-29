#!/bin/bash
# because windows(tm) is gener confused about directory slashes,
# we have to do a post process on the makefiles to correct them

qmake

pushd common
sed -i 's/C:\\Qt\\5.15.0\\mingw81_32\\bin\\moc.exe/moc.exe/g' Makefile.Debug
sed -i 's/C:\\Qt\\5.15.0\\mingw81_32\\bin\\moc.exe/moc.exe/g' Makefile.Release
popd 

make 
