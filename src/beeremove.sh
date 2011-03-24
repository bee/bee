#!/bin/bash

###############################################################################
###############################################################################
###############################################################################

VERSION=0.1

BEE_SYSCONFDIR=/etc/bee
BEE_DATADIR=/usr/share

: ${DOTBEERC:=${HOME}/.beerc}
if [ -e ${DOTBEERC} ] ; then
    . ${DOTBEERC}
fi

: ${BEEFAULTS:=${BEE_SYSCONFDIR}/beerc}
if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${BEE_METADIR=${BEE_DATADIR}/bee}
: ${BEE_REPOSITORY_PREFIX=/usr/src/bee}

: ${BEE_TMP_TMPDIR:=/tmp}
: ${BEE_TMP_BUILDROOT:=${BEE_TMP_TMPDIR}/beeroot-${LOGNAME}}

: ${BEE_SKIPLIST=${BEE_SYSCONFDIR}/skiplist}

# copy file.bee to ${BEE_REPOSITORY_BEEDIR} after successfull build
: ${BEE_REPOSITORY_BEEDIR:=${BEE_REPOSITORY_PREFIX}/bees}

# directory where (new) bee-pkgs are stored
: ${BEE_REPOSITORY_PKGDIR:=${BEE_REPOSITORY_PREFIX}/pkgs}

# directory where copies of the source+build directories are stored
: ${BEE_REPOSITORY_BUILDARCHIVEDIR:=${BEE_REPOSITORY_PREFIX}/build-archives}

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
        return
    fi
    
    # pattern is a pkg in BEE_METADIR
    if [ -d ${BEE_METADIR}/${search} ] ; then
        do_remove ${BEE_METADIR}/${search}
        return
    fi
    
    # pattern is no installed pkg
    # show all pkgs that match pattern
    PKGS=$(find ${BEE_METADIR} -mindepth 1 -maxdepth 1 -iregex "${BEE_METADIR}.*${search}.*")
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
        RELPKG=$(egrep "file=$s(|//.*)$" ${BEE_METADIR}/*/FILES)
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
    if [ -f ${pkg}/FILES ] ; then
        ${NOOP:+echo} rm -vfr ${pkg}
    fi
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
            if [ -z "${1}" ] ; then
                usage
                exit 1
            fi
            pkg_remove_all "$@"
            break
            ;;
    esac
done
