#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="glib-compile-schemas"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

for dir in ${XDG_DATA_DIRS//:/ } ; do
    schema_dir=${dir}/glib-2.0/schemas
    if grep -q ${schema_dir} ${BEE_METADIR}/${pkg}/FILES ; then
        case "${action}" in
            "post-install")
                rm -f ${schema_dir}/gschemas.compiled
                glib-compile-schemas ${schema_dir}
                ;;
            "pre-remove")
                rm -f ${schema_dir}/gschemas.compiled
                ;;
            "post-remove")
                glib-compile-schemas ${schema_dir}
                ;;
        esac
    fi
done
