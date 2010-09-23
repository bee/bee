#!/bin/bash
set -e

pname=$1
surl=$2

if [ -z ${pname} ] ; then 
    echo "$0 -i <pkgname> [url]"
    echo "$0 -i <url>"
    exit
fi

if [ -z ${surl} ] ; then
    surl=${pname}
    pname=$(basename $(basename ${surl} .tar.bz2) .tar.gz)
    if [ ${pname} = ${surl} ] ; then
        surl=""
    else
        pname=${pname}-0
    fi
fi

echo "creating ${pname}.bee with default SRCURL='${surl}'"

cat >${pname}.bee <<EOF
#!/bin/env beesh

PGRP=( uncategorized )

SRCURL[0]="${surl}"

PATCHURL[0]=""

# EXCLUDE=""

mee_patch() {
    bee_patch
}

mee_configure() {
    bee_configure
}

mee_build() {
    bee_build
}

mee_install() {
    bee_install
}

EOF
    chmod 755 ${pname}.bee
