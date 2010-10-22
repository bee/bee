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

: ${BEEMETADIR:=/usr/share/bee}
: ${BEEPKGSTORE:=/usr/src/bee/pkgs}


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
        do_query ${f}
    done
}

do_query() {
    file=$1
    
    for i in ${BEEMETADIR}/* ; do
        if grep -q ${file} ${i}/FILES ; then
            pkgs="${pkgs:+${pkgs} }${i}"
        fi
    done
    echo "'${file}' is related to .."
    for p in ${pkgs} ; do
        echo "  ${p}"
    done
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
