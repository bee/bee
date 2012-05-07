#!/bin/bash
#
# info dir hook
#
# Copyright (C) 2009-2012
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#       and other bee developers
#
# This file is part of bee.
#
# bee is free software; you can redistribute it and/or modify
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
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

: ${INSTALLINFO:=install-info}

if ! which ${INSTALLINFO} >/dev/null 2>&1 ; then
    exit 0
fi

if [ ! -r "${BEE_METADIR}/${pkg}/META" ] ; then
    exit 0
fi

. ${BEE_METADIR}/${pkg}/META

if [ ! "${BEEMETAFORMAT}" -ge 2 ] ; then
    exit 0
fi

: ${DIRFILE:=${PKG_INFODIR}/dir}

case "${action}" in
    "post-install")
        for i in $(grep -o "${PKG_INFODIR}.*\.info.*" ${BEE_METADIR}/${pkg}/FILES 2>/dev/null) ; do
            echo "adding ${i##*/} to ${DIRFILE}"
            ${INSTALLINFO} ${i} ${DIRFILE} >/dev/null
        done
        ;;
    "pre-remove")
        for i in $(grep -o "${PKG_INFODIR}.*\.info.*" ${BEE_METADIR}/${pkg}/FILES 2>/dev/null) ; do
            echo "removing ${i##*/} from ${DIRFILE}"
            ${INSTALLINFO} --delete ${i} ${DIRFILE} >/dev/null
        done
        ;;
esac