#!/bin/bash
set -e

: ${TEMPLATEPATH:=/etc/bee/templates/}

usage() {
    echo "usage.."
    echo "$0 [options] <url>"
    echo "$0 [options] <pkgname> [url]"
    echo
    echo "options.."
    echo "  -h                .. show this help"
    echo "  -t [xorg|default] .. specify template used for beefile"
}

initialize() {
    pname=$1
    surl=$2
    if [ -z ${surl} ] ; then
        surl=${pname}
        pname=$(basename $(basename ${surl} .tar.bz2) .tar.gz)
        if [ ${pname} = ${surl} ] ; then
            surl=""
        else
            pname=${pname}-0
        fi
    fi

    echo "creating ${pname}.bee with TEMPLATE='${TEMPLATE}' and SRCURL='${surl}'"

    cp ${TEMPLATEPATH}${TEMPLATE} ${pname}.bee

    sed -e "s,SRCURL.*,SRCURL[0]=\"${surl}\"," -i ${pname}.bee

    chmod 755 ${pname}.bee
}

options=$(getopt -n beeinit \
                 -o ht: \
                 --long help,template \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

while true ; do
  case "$1" in
    -h|--help)
      usage
      exit 0
    ;;
    -t|--template)
      shift
      TEMPLATE=$1
      if [ ! -e ${TEMPLATEAPTH}${TEMPLATE} ] ; then
          echo "template '${TEMPLATE}' doesn't exist in '${TEMPLATEPATH}' .. 'default' template used instead .."
          TEMPLATE=default
      fi
      shift
      ;;
    *)
      shift
      if [ -z ${1} ] ; then
           usage
           exit 1
      fi
      : ${TEMPLATE:=default}
      initialize ${@}
      exit 0;
      ;;
  esac
done
