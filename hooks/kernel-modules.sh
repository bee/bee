#!/bin/bash
#
action=${1}
pkg=${2}
content=${3}
: ${content:=${BEE_METADIR}/${pkg}/CONTENT}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

: ${DEPMOD:=depmod}

if [ ! type -p ${DEPMOD} >/dev/null 2>&1 ; then
    exit 0
fi

if [ ! -r "${BEE_METADIR}/${pkg}/META" ] ; then
    exit 0
fi

. ${BEE_METADIR}/${pkg}/META

if [ ! "${BEEMETAFORMAT:=0}" -ge 2 ] ; then
    exit 0
fi

case "${action}" in
    "post-install"|"post-remove")
        for ver in  $(grep -Po  ':file=/lib/modules/\K([^/]+)(?=/)' ${content}|sort -u) ; do
            echo "$DEPMOD $ver"
            test -e /lib/modules/$ver/modules.builtin && $DEPMOD $ver
        done
        ;;
esac
