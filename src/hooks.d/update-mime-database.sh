#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

if ! which update-mime-database >/dev/null 2>&1 ; then
    exit 0
fi

for dir in ${XDG_DATA_DIRS//:/ } ; do
    mime_dir=${dir}/mime
    if grep -q "file=${mime_dir}/packages" ${BEE_METADIR}/${pkg}/FILES ; then
        case "${action}" in
            "post-install")
                update-mime-database ${mime_dir}
                ;;
            "post-remove")
                update-mime-database ${mime_dir}
                ;;
        esac
    fi
done
