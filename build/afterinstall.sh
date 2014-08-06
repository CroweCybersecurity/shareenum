#!/bin/sh
SRC="/usr/local/shareenum/bin/shareenum"
DST="/usr/bin/shareenum"
if [ -f $DST ]; then
	rm $DST
fi
ln -s $SRC $DST
