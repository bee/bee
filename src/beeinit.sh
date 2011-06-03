#!/bin/bash
set -e

BEE_SYSCONFDIR=/etc/bee

: ${BEE_TEMPLATEDIR:=${BEE_SYSCONFDIR}/templates}

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

	# fix sourceforge urls for automatic pkgname generation
	if [[ $surl = *://sourceforge.net/*/files/*/download ]] ; then
	    # strip /files directory
	    surl=${surl/\/files\///}

	    # rename directory /projects to /project
	    surl=${surl/\/projects\///project/}

            # replace hostname sourceforge.net -> downloads.sourceforge.net
            surl=${surl/\/\/sourceforge.net\////downloads.sourceforge.net/}

	    # strip /download basename
	    surl=${surl%/download}

	    pname=${surl}
	fi

        pname=$(basename ${pname} .bz2)
        pname=$(basename ${pname} .gz)
        pname=$(basename ${pname} .tar)
        pname=$(basename ${pname} .tgz)
        pname=$(basename ${pname} .tbz2)
        pname=$(basename ${pname} .zip)
        pname=$(basename ${pname} .bee)
        
        if [ ${pname} = ${surl} ] ; then
            surl=""
        else
            pname=${pname}-0
        fi
    fi
    
    beefile="${pname}.bee"

    if [ -e ${beefile} ] && [ "$OPT_FORCE" != "yes" ] ; then
        echo "${beefile} already exists .. use option -f to overwrite .."
        exit 1
    fi

    
    if [ -r ${BEE_TEMPLATEDIR}/${TEMPLATE} ] ; then
        echo "creating ${beefile} with TEMPLATE='${TEMPLATE}' and SRCURL='${surl}'"
        cp ${BEE_TEMPLATEDIR}/${TEMPLATE} ${pname}.bee
    else
        echo "creating ${beefile} with SRCURL='${surl}'"
        cat >${pname}.bee <<-"EOT"
	#!/bin/env beesh
	
	@SRCURL@
	
	PATCHURL[0]=""
	
	@BEE_CONFIGURE@
	
	# EXCLUDE=""
	
	@DEFAULT_PREFIX_VARS@
	
	mee_extract() {
		bee_extract ${@}
	}
	
	mee_patch() {
		bee_patch ${@}
	}
	
	mee_configure() {
		bee_configure @DEFAULT_CONFIGURE_OPTIONS@
	}
	
	mee_build() {
		bee_build
	}
	
	mee_install() {
		bee_install
	}
	EOT
    fi
    
    sed -e "s,@SRCURL@,SRCURL[0]=\"${surl}\"," \
        -i ${beefile}
    
    sed -e "s,@DEFAULT_CONFIGURE_OPTIONS@,${DEFAULT_CONFIGURE_OPTIONS}," \
        -i ${beefile}
    
    for i in prefix eprefix bindir sbindir libexecdir sysconfdir \
              localstatedir sharedstatedir libdir includedir datarootdir \
              datadir infodir mandir docdirlocaledir ; do
        I=$(tr a-z A-Z <<<"${i}")
        eval dir=\$OPT_${i}
        DEFAULT_PREFIX_VARS="${DEFAULT_PREFIX_VARS:+${DEFAULT_PREFIX_VARS}}${dir:+${DEFAULT_PREFIX_VARS:+\n}}${dir:+${I}=${dir}}"
        unset dir
    done
    
    sed -e "s,@DEFAULT_PREFIX_VARS@,${DEFAULT_PREFIX_VARS}," \
        -i ${beefile}
    
    if [ "${CONFIGURE_BEEHAVIOR}" ] ; then
        sed -e "s,@BEE_CONFIGURE@,BEE_CONFIGURE=${CONFIGURE_BEEHAVIOR}," \
            -i ${beefile}
    else
        sed -e "s,@BEE_CONFIGURE@,# BEE_CONFIGURE=compat," \
            -i ${beefile}
    fi

    chmod 755 ${beefile}
}

options=$(getopt -n beeinit \
                 -o ht:f \
                 --long help,template:,force \
                 --long configure: \
                 --long prefix:,eprefix:,bindir:,sbindir:,libexecdir: \
                 --long sysconfdir:,localstatedir:,sharedstatedir: \
                 --long libdir:,includedir:,datarootdir:,datadir: \
                 --long infodir:,mandir:,docdir:,localedir: \
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
            TEMPLATE=${2}
            shift 2
            
            if [ ! -e ${BEE_TEMPLATEDIR}/${TEMPLATE} ] ; then
                echo "ignoring non-existant template '${BEE_TEMPLATEDIR}/${TEMPLATE}' .."
                unset TEMPLATE
            fi
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
