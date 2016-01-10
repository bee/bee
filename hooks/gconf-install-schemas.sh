#!/bin/bash
#
# gconf install schemas hook
#
# Copyright (C) 2009-2016
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#       Marius Tolzmann <m@rius.berlin>
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
content=${3}
: ${content:=${BEE_METADIR}/${pkg}/CONTENT}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

: ${GCONFTOOL:=gconftool-2}

if ! type -p ${GCONFTOOL} >/dev/null 2>&1 ; then
    exit 0
fi

case "${action}" in
    "post-install")
        for s in $(grep -o "/.*gconf.*\.schemas" ${content} 2>/dev/null) ; do
            echo "installing schema '${s##*/}'"
            ${GCONFTOOL} --install-schema-file ${s} >/dev/null
        done
    ;;
esac
