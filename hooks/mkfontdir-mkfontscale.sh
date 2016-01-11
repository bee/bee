#!/bin/bash
#
# font hook
#
# Copyright (C) 2009-2016
#       Marius Tolzmann <m@rius.berlin>
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

if ! type -p mkfontscale mkfontdir >/dev/null 2>&1 ; then
    exit 0
fi

function clean_font_dirs() {
    local font_base_dir=${1}

    font_dirs=$(find ${font_base_dir} -mindepth 1 -type d)
    for d in ${font_dirs} ; do
        rm -f ${d}/fonts.{scale,dir}
    done
}

function update_fonts() {
    local font_base_dir=${1}

    font_dirs=$(find ${font_base_dir} -mindepth 1 -type d)
    for d in ${font_dirs} ; do
        mkfontscale ${d}
        mkfontdir ${d}
    done
}

for dir in ${XDG_DATA_DIRS//:/ } ; do
    font_base_dir=${dir}/fonts
    if grep -q "file=${font_base_dir}" ${content} ; then
        case "${action}" in
            "post-install")
                clean_font_dirs ${font_base_dir}
                update_fonts ${font_base_dir}
                ;;
            "pre-remove")
                clean_font_dirs ${font_base_dir}
                ;;
            "post-remove")
                update_fonts ${font_base_dir}
                ;;
        esac
    fi
done
