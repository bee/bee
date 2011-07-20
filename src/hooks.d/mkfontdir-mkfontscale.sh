#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

if ! which mkfontscale mkfontdir >/dev/null 2>&1 ; then
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
    if grep -q ${font_base_dir} ${BEE_METADIR}/${pkg}/FILES ; then
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
