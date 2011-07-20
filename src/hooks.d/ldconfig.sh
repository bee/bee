#!/bin/bash
action=${1}
pkg=${2}

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: cannot call $0 from the outside of bee .."
    exit 1
fi

binaries="ldconfig"
for bin in ${binaries} ; do
    if [ -z "$(which ${bin} 2>/dev/null)" ] ; then
        exit 0
    fi
done

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
