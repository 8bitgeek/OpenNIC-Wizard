#!/usr/bin/bash

DEST_BIN=/usr/local/bin
DEST_ETC=/usr/local/etc
DEST_LIB=/usr/local/lib/opennic 

mkdir -p $DEST_LIB

cp server/opennicd $DEST_BIN/
cp client/opennicui $DEST_BIN/

cp client/images/opennic.png $DEST_LIB/

cp server/bootstrap.domains $DEST_ETC/
cp server/bootstrap.t1 $DEST_ETC/

