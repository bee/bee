#!/bin/bash
#
# mandb hook
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
content=${3}
: ${content:=${BEE_METADIR}/${pkg}/CONTENT}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

if ! type -p mandb >/dev/null 2>&1 ; then
    exit 0
fi

if [ -r "${BEE_METADIR}/${pkg}/META" ] ; then
    . ${BEE_METADIR}/${pkg}/META
fi

: ${man_dirs:=${PKG_MANDIR}}
: ${man_dirs:=${XDG_DATA_DIRS//:/\/man:}/man}

for man_dir in $(beeuniq ${man_dirs//:/ }) ; do
    case "${action}" in
        "post-remove"|"post-install")
            if grep -q "file=${man_dir}" ${content} ; then
                if [ ! -d ${man_dir} ]; then
                    continue
                fi

                nfiles=$(ls ${man_dir}/ | head -2 | grep -c -v index.db)
                if [ "${nfiles}" -gt 0 ] ; then
                    rm -f ${man_dir}/index.db
                else
                    echo "updating manual index cache for ${man_dir} .."
                    mandb -q ${man_dir}
                fi
            fi
            ;;
    esac
done
