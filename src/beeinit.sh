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
    echo "  -f                .. force overwriting of beefile"
}

initialize() {
    # get arguments starting with '--'
    while [ -n "${1}" ] ; do
        if [ "${1:0:2}" == "--" ] ; then
            DEFAULT_CONFIGURE_OPTIONS="${DEFAULT_CONFIGURE_OPTIONS:+${DEFAULT_CONFIGURE_OPTIONS} }${1}"
        else
            PP="${PP:+${PP} }${1}"
        fi
        shift
    done

    # set positional parameters to arguments not starting with '--'
    set -- ${PP}
    pname=$1
    surl=$2
    if [ -z "${surl}" ] ; then
        surl=${pname}
        pname=$(basename $(basename ${surl} .tar.bz2) .tar.gz)
        if [ ${pname} = ${surl} ] ; then
            surl=""
        else
            pname=${pname}-0
        fi
    fi
    

    if [ -e ${pname}.bee ] && [ "$OPT_FORCE" != "yes" ] ; then
        echo "${pname}.bee already exists .. use option -f to overwrite .."
        exit 1
    fi

    echo "creating ${pname}.bee with TEMPLATE='${TEMPLATE}' and SRCURL='${surl}'"
    if [ -e ${TEMPLATEPATH}${TEMPLATE} ] ; then
        cp ${TEMPLATEPATH}${TEMPLATE} ${pname}.bee
    else
        cat >${pname}.bee<<"EOT"
#!/bin/env beesh
@SRCURL@
@DEFAULT_PREFIX_VARS@

@BEE_CONFIGURE@

mee_configure() {
    bee_configure @DEFAULT_CONFIGURE_OPTIONS@
}
EOT
    fi
    
    sed -e "s,@SRCURL@,SRCURL[0]=\"${surl}\"," -i ${pname}.bee
    
    sed -e "s,@DEFAULT_CONFIGURE_OPTIONS@,${DEFAULT_CONFIGURE_OPTIONS}," -i ${pname}.bee
    
    for i in prefix eprefix bindir sbindir libexecdir sysconfdir localstatedir sharedstatedir libdir includedir datarootdir datadir infodir mandir docdirlocaledir ; do
        I=$(echo ${i} | tr a-z A-Z)
        eval dir=\$OPT_${i}
        DEFAULT_PREFIX_VARS="${DEFAULT_PREFIX_VARS:+${DEFAULT_PREFIX_VARS}}${dir:+${DEFAULT_PREFIX_VARS:+\n}}${dir:+${I}=${dir}}"
        unset dir
    done
    sed -e "s,@DEFAULT_PREFIX_VARS@,${DEFAULT_PREFIX_VARS}," -i ${pname}.bee
    
    if [ "${CONFIGURE_BEEHAVIOR}" ] ; then
        sed -e "s,@BEE_CONFIGURE@,BEE_CONFIGURE=${CONFIGURE_BEEHAVIOR}," -i ${pname}.bee
    else
        sed -e "s,@BEE_CONFIGURE@,# BEE_CONFIGURE=compat," -i ${pname}.bee
    fi

    chmod 755 ${pname}.bee
}

options=$(getopt -n beeinit \
                 -o ht:f \
                 --long help,template:,force \
                 --long configure: \
                 --long prefix:,eprefix:,bindir:,sbindir:,libexecdir:,sysconfdir: \
                 --long localstatedir:,sharedstatedir:,libdir:,includedir: \
                 --long datarootdir:,datadir:,infodir:,mandir:,docdir:,localedir: \
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
                echo "ignoring non-existant template '${TEMPLATEPATH}${TEMPLATE}' .."
                unset TEMPLATE
            fi
            shift
            ;;
        -f|--force)
            shift
            OPT_FORCE="yes"
            ;;
        --configure)
            case "${2}" in 
                compat|none)
                    CONFIGURE_BEEHAVIOR=${2}
                    ;;
                *)
                    unset CONFIGURE_BEEHAVIOR
                    ;;
            esac
            shift 2
            ;;
        --prefix|\
        --eprefix|\
        --bindir|\
        --sbindir|\
        --libexecdir|\
        --sysconfdir|\
        --localstatedir|\
        --sharedstatedir|\
        --libdir|\
        --includedir|\
        --datarootdir|\
        --datadir|\
        --infodir|\
        --mandir|\
        --docdir|\
        --localedir)
            eval OPT_${1:2}="${2}"
            shift 2
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
