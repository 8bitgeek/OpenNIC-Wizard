#!/usr/bin/bash
#
# This file is a part of OpenNIC Wizard
# Copywrong (c) 2012-2022 Mike Sharkey
# ----------------------------------------------------------------------------
# "THE BEER-WARE LICENSE" (Revision 1776):
# <mike@8bitgeek.net> wrote this file.
# As long as you retain this notice you can do whatever you want with this
# stuff. If we meet some day, and you think this stuff is worth it,
# you can buy me a beer in return. ~ Mike Sharkey
# ----------------------------------------------------------------------------
#
# Roll up an debian package... 
#

OPENNIC_SERVER_NAME=opennicd
OPENNIC_CLIENT_NAME=opennicui

SERVER_SRC_DIR=./server 
CLIENT_SRC_DIR=./client

SERVER_SRC=$SERVER_SRC_DIR/$OPENNIC_SERVER_NAME
CLIENT_SRC=$CLIENT_SRC_DIR/$OPENNIC_CLIENT_NAME

TARGET_ETC=/usr/local/etc
TARGET_BIN=/usr/local/bin

WORKING_DIR=./debian
WORKING_DIR_ETC=$WORKING_DIR/$TARGET_ETC
WORKING_DIR_BIN=$WORKING_DIR/$TARGET_BIN

make_working_dir()
{
    echo "make_working_dir"
    mkdir -p $WORKING_DIR_ETC
    mkdir -p $WORKING_DIR_BIN
}

clean_working_dir()
{
    echo "clean_working_dir"
    rm -rf $WORKING_DIR
}

copy_one_file()
{
    $SRC=$1
    $DST=$2
    echo "$SRC .. $DST"
    cp $SRC $DST
}

copy_to_working_dir()
{
    echo "copy_to_working_dir"

    copy_one_file $SERVER_SRC $WORKING_DIR_BIN/
    copy_one_file $CLIENT_SRC $WORKING_DIR_BIN/
    copy_one_file $CLIENT_SRC_DIR/images/opennic.png $WORKING_DIR_ETC/

    copy_one_file $SERVER_SRC_DIR/bootstrap.domains $WORKING_DIR_ETC/
    copy_one_file $SERVER_SRC_DIR/bootstrap.t1 $WORKING_DIR_ETC/
}

make_deb_package()
{
    echo "make_deb_package"
    copy_to_working_dir
}

make_working_dir
make_dep_package
clean_working_dir