#!/bin/bash

: ${METADIR:=/usr/share/bee}

echo "checking '${METADIR}' for obsolete packages .."
for f in ${METADIR}/* ; do
    if [ -d ${f} ] && [ -e ${f}/BUILD ] ; then
        p=$(basename $(basename $(basename $(basename ${f} .any) .noarch) .x86_64) .i686)
        mv -v ${f}/{BUILD,${p}.bee}
    fi
done
