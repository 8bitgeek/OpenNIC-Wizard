#!/usr/bin/bash

DEST_BIN=/usr/local/bin
DEST_ETC=/usr/local/etc
DEST_LIB=/usr/local/lib/opennic 

mkdir -p $DEST_LIB

cp server/opennicwizd $DEST_BIN/
cp client/opennicwizui $DEST_BIN/

cp client/images/opennic.png $DEST_LIB/

cp etc/bootstrap.domains $DEST_ETC/
cp etc/bootstrap.t1 $DEST_ETC/

