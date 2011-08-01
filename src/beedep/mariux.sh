#!/bin/bash

export BEE_METADIR=/usr/share/bee

PACKAGES=`bee list`

# print which files the package provides
#for i in $PACKAGES; do
#    echo "[$i]"
#    if [ ! -e /usr/share/bee/$i/FILES ]; then
#        continue
#    fi
#
#    for j in `cat /usr/share/bee/$i/FILES`; do
#        FILE=`echo $j | sed -e 's@.*file=@@'`
#        echo -e "\tprovides = $FILE"
#    done
#    echo
#done

# print information about ELF files
#for i in $PACKAGES; do
#    if [ ! -e /usr/share/bee/$i/FILES ]; then
#        continue
#    fi
#
#    for j in `cat /usr/share/bee/$i/FILES`; do
#        # get provided ELF objects
#        FILE=`echo $j | sed -e 's@.*file=@@'`
#
#        # check if FILE is an ELF
#        file -b $FILE | grep ELF 1>/dev/null
#
#        if [ $? == "0" ]; then
#            echo "[$FILE]"
#            for k in `readelf -d $FILE | grep SONAME | sed -e 's@.*\[@@;s@\].*@@'`; do
#                echo -e "\tprovides = $k"
#            done
#
#            # get needs of ELF object
#            for k in `readelf -d $FILE | grep NEEDED | sed -e 's@.*\[@@;s@\].*@@'`; do
#                echo -e "\tneeds = $k"
#            done
#
#            for k in `ldd $FILE | awk '{print $1}'`; do
#                echo -e "\tneeds = $k"
#            done

            # print type
#        fi
#    done
#    echo
#done

for i in `bee list`; do
    bee check -d $i
    echo
done
