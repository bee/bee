#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="update-mime-database"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

for dir in ${XDG_DATA_DIRS//:/ } ; do
    mime_dir=${dir}/mime
    if grep -q "${mime_dir}/packages" ${BEE_METADIR}/${pkg}/FILES ; then
        case "${action}" in
            "post-install")
                update-mime-database -V ${mime_dir}
                ;;
            "post-remove")
                update-mime-database -V ${mime_dir}
                ;;
        esac
    fi
done
