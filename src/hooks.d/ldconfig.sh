#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

if [ ${UID} -ne 0 ] ; then
    exit 0
fi

if ! which ldconfig >/dev/null 2>&1 ; then
    exit 0
fi

if grep -q "/lib/" ${BEE_METADIR}/${pkg}/FILES ; then
    case "${action}" in
        "post-install")
            ldconfig
            ;;
        "post-remove")
            ldconfig
            ;;
    esac
fi
