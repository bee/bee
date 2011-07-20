#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="gtk-update-icon-cache"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

for dir in ${XDG_DATA_DIRS//:/ } ; do
    icon_base_dir=${dir}/icons
    for line in $(grep -h "${icon_base_dir}/.*/index.theme" ${BEE_METADIR}/${pkg}/FILES) ; do
        eval $(beesep ${line})
        icon_dir=${file%%/index.theme}
        case "${action}" in
            "post-install")
                rm -f ${icon_dir}/icon-theme.cache
                gtk-update-icon-cache -f ${icon_dir}
                ;;
            "pre-remove")
                rm -f ${icon_dir}/icon-theme.cache
                ;;
        esac
    done
done
