#!/usr/bin/bash

DEST_BIN=/usr/local/bin
DEST_ETC=/usr/local/etc

cp server/OpenNICServer $DEST_BIN/
cp client/OpenNIC $DEST_BIN/
cp client/images/opennic.png $DEST_ETC/

cp server/bootstrap.domains $DEST_ETC/
cp server/bootstrap.t1 $DEST_ETC/

