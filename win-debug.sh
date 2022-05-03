#!/usr/bin/bash

SERVER=server/debug
QTBIN=/cygdrive/c/Qt/5.15.0/mingw81_64/bin

cp $QTBIN/Qt5Core.dll $SERVER/
cp $QTBIN/Qt5Network.dll $SERVER/
cp $QTBIN/Qt5Script.dll $SERVER/
cp $QTBIN/libgcc_s_seh-1.dll $SERVER/
cp $QTBIN/libwinpthread-1.dll $SERVER/
cp $QTBIN/libstdc++-6.dll $SERVER/

cd $SERVER/
gdb opennicd.exe 


