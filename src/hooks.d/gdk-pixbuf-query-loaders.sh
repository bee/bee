#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="gdk-pixbuf-query-loaders"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

cachefile=$(pkg-config --variable=gdk_pixbuf_cache_file gdk-pixbuf-2.0)
if grep -q "gdk-pixbuf-2.0/.*/loaders/.*\.(so|a)" ${BEE_METADIR}/${pkg}/FILES ; then
    case "${action}" in
        "post-install")
            rm -f ${cachefile}
            gdk-pixbuf-query-loaders --update-cache
            ;;
        "pre-remove")
            rm -f ${cachefile}
            ;;
        "post-remove")
            gdk-pixbuf-query-loaders --update-cache
            ;;
    esac
fi
