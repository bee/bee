#!/bin/bash

###############################################################################
###############################################################################
###############################################################################

VERSION=0.1

: ${BEEFAULTS=/etc/bee/beerc}

if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${BEEMETADIR:=/usr/share/bee}

pkg_remove_all() {
    for pkg in ${@} ; do
        pkg_remove ${pkg}
    done
}

pkg_remove() {
    search=$1
    
    # pattern is absolute path to pkg
    if [ -d ${search} ] ; then
        do_remove "${search}"
        exit
    fi
    
    # pattern is a pkg in BEEMETADIR
    if [ -d ${BEEMETADIR}/${search} ] ; then
        do_remove ${BEEMETADIR}/${search}
        exit
    fi
    
    # pattern is no installed pkg
    # show all pkgs that match pattern
    PKGS=$(find ${BEEMETADIR} -mindepth 1 -maxdepth 1 -iregex "${BEEMETADIR}.*${search}.*")
    echo "${search} matches following packages .."
    for p in ${PKGS} ; do
        [ -d $p ] && echo " -> $(basename $p)"
    done
}

do_remove() {
    pkg=$1
    
    FILES=$(beefind.pl --dump ${pkg}/FILES)
    
    # removing files
    for f in $FILES ; do
        # test for other pkg
        s=$(echo $f | sed -e "s,[\`|&^$.+?(){}],\\\&,g" -e "s,\[,\\\&,g" -e "s,\],\\\&,g")
        RELPKG=$(egrep "file=$s(|//.*)$" ${BEEMETADIR}/*/FILES)
        if [ 1 -eq $(echo $RELPKG | wc -w) ] ; then
            #check for directories
            if [ -d $f ] ; then
                DIR="$f $DIR"
            else
                ${NOOP:+echo} rm -vf $f
            fi
        else
          echo "cannot remove $f .. is related to other pkgs"
        fi
    done
    
    #removing directories
    for d in $DIR ; do
        if [ -z "$(ls $d)" ] ; then
            ${NOOP:+echo} rmdir -v $d
        fi
    done
    
    # create bee-filename
    BEE=$(basename $(echo ${pkg} | sed -e "s,\(.*\)-\(.*\)-\(.*\)\..*,\1-\2-\3.bee," - ))
    
    #cleaning up meta directory
    for f in FILES META ${BEE}; do
        ${NOOP:+echo} rm -vf ${pkg}/$f
    done
    ${NOOP:+echo} rmdir -v ${pkg}
}

usage() {
    echo "usage: $0 <option> <package>"
    echo
    echo "options:"
    echo "    -h --help .. prints this"
    echo "    --noop    .. prints what would happen"
}

options=$(getopt -n bee_remove \
                 -o h \
                 --long noop,help \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

while true ; do
    case "$1" in
        --noop)
            shift
            NOOP=1
            ;;
        -h|--help)
            usage
            exit
            ;;
        --)
            shift
            pkg_remove_all "$@"
            break
            ;;
    esac
done
