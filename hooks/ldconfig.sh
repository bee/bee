#!/bin/bash
#
# ldconfig hook
#
# Copyright (C) 2009-2012
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#       Tobias Dreyer <dreyer@molgen.mpg.de>
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

if [ ${UID} -ne 0 ] ; then
    exit 0
fi

if ! which ldconfig >/dev/null 2>&1 ; then
    exit 0
fi

if grep -q "/lib/" ${BEE_METADIR}/${pkg}/CONTENT ; then
    case "${action}" in
        "post-install")
            ldconfig
            ;;
        "post-remove")
            ldconfig
            ;;
    esac
fi
