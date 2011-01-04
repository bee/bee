#!/bin/bash
#
# bee-query - query bee metadata
# Copyright (C) 2009-2010 
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

VERSION=0.1

: ${DOTBEERC:=${HOME}/.beerc}
if [ -e ${DOTBEERC} ] ; then
    . ${DOTBEERC}
fi

: ${BEEFAULTS:=/etc/bee/beerc}
if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${defaultBEEMETADIR:=/usr/share/bee}
: ${BEEMETADIR:=${defaultBEEMETADIR}}
: ${BEEPKGSTORE:=/usr/src/bee/pkgs}

BEEMETAPATH=( $(for d in "${defaultBEEMETADIR}" "${BEEMETADIR}" ; do echo $d ; done | sort | uniq) )


##### usage ###################################################################
usage() {
    echo "bee-query v${VERSION} 2009-2010"
    echo ""
    echo "  by Tobias Dreyer and Marius Tolzmann <{dreyer,tolzmann}@molgen.mpg.de>"
    echo ""
    echo " Usage: $0 <file> [ <file> .. ]"
}

query() {
    list=$@
    
    for f in "${list[@]}" ; do
        local -i done=0
        
        # check if $f is pkg   --> list related files
        # otherwise $f is file --> list pkg
        pkgname=$(basename $f)
        for metadir in "${BEEMETAPATH[@]}" ; do
            if [ -d "${metadir}/${pkgname}" ] ; then
                get_files "${metadir}/${pkgname}"
                done=1
            fi
        done
        
        [ $done -eq 1 ] && exit $?
        
        for metadir in "${BEEMETAPATH[@]}" ; do
            get_pkgs "${metadir}" "${f}"
        done
        
    done
}

get_files() {
    pkg=${1}
    
    ff="${pkg}/FILES"
    if [ -e "${ff}" ] ; then
        for line in $(cat ${ff}) ; do
            beefind2filename $line
        done
    fi
}

get_pkgs() {
    dir="${1}"
    file="${2}"
    
    for pkg in ${dir}/* ; do
        if [ -r "${pkg}/FILES" ] && egrep -q "file=.*${file}" ${pkg}/FILES ; then
            echo $(basename ${pkg})
            for line in $(egrep "file=.*${file}" ${pkg}/FILES) ; do
                filename=$(beefind2filename $line)
                [ "$(echo ${filename} | grep ${file})" != "" ] && echo "  ${filename}"
            done
        fi
    done
}

beefind2filename() {
    line=$1
    
    OLDIFS=${IFS}
    IFS=":"
    eval $line
    echo ${file%%//*}
    unset md5 mode nlink uid gid size mtime file
    IFS=${OLDIFS}
}



###############################################################################
###############################################################################
###############################################################################

options=$(getopt -n bee-query \
                 -o h \
                 --long help \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

while true ; do
  case "$1" in
    -h|--help)
      usage
      exit 0
    ;;
    *)
      shift
      if [ -z ${1} ] ; then
           usage
           exit 1
      fi
      query ${@}
      exit 0;
      ;;
  esac
done
