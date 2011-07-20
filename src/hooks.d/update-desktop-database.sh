#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="update-desktop-database"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

for dir in ${XDG_DATA_DIRS//:/ } ; do
    desktop_dir=${dir}/applications
    if grep -q "${desktop_dir}/.*\.desktop" ${BEE_METADIR}/${pkg}/FILES ; then
        case "${action}" in
            "post-install")
                rm -f ${desktop_dir}/mimeinfo.cache
                update-desktop-database ${desktop_dir}
                ;;
            "pre-remove")
                rm -f ${desktop_dir}/mimeinfo.cache
                ;;
            "post-remove")
                update-desktop-database ${desktop_dir}
                ;;
        esac
    fi
done
