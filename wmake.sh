#!/bin/bash
cd common
qmake 
make $1
cd  ..
cd client 
qmake 
make $1
cd ..
cd server
qmake 
make $1
cd ..
