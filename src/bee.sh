#!/bin/bash

BEE_SYSCONFDIR=/etc
BEE_DATADIR=/usr/share
BEE_LIBEXECDIR=/usr/lib/bee

# XDG defaults as defined in xdg base directory specification
: ${XDG_CONFIG_HOME:=${HOME}/.config}
: ${XDG_CONFIG_DIRS:=/etc/xdg}
: ${XDG_DATA_HOME:=${HOME}/.local/share}
: ${XDG_DATA_DIRS:=/usr/local/share:/usr/share}

function pathremove() {
    local IFS=':'
    local NEWPATH
    local DIR
    local PATHVARIABLE=${2:-PATH}
    for DIR in ${!PATHVARIABLE} ; do
        if [ "$DIR" != "$1" ] ; then
            NEWPATH=${NEWPATH:+$NEWPATH:}$DIR
        fi
    done
    export $PATHVARIABLE="$NEWPATH"
}

function pathprepend() {
    pathremove $1 $2
    local PATHVARIABLE=${2:-PATH}
    export $PATHVARIABLE="$1${!PATHVARIABLE:+:${!PATHVARIABLE}}"
}

function pathappend() {
    pathremove $1 $2
    local PATHVARIABLE=${2:-PATH}
    export $PATHVARIABLE="${!PATHVARIABLE:+${!PATHVARIABLE}:}$1"
}

print_msg() {
    echo -e "${COLOR_BRACKET}[${COLOR_BRCONTENT}BEE${COLOR_BRACKET}] ${@}"
}

print_info() {
    print_msg "${COLOR_INFO}${@}${COLOR_NORMAL}"
}

print_error() {
    print_msg "${COLOR_ERROR}${@}${COLOR_NORMAL}"
}

###############################################################################
###############################################################################
###############################################################################

function init_config() {
    local IFS=":${IFS}"     # add ':' to IFS

    # default XDG data dir to write data to
    BEE_XDG_DATADIR=${BEE_DATADIR}

    # handle deprecated config locations
    : ${DOTBEERC:=${HOME}/.beerc}
    if [ -r ${DOTBEERC} ] ; then
        print_error \
            "ERROR: support for ~/.beerc is" \
            " deprecated please move it to" \
	    " ${XDG_CONFIG_HOME}/bee/beerc"
        exit 1
    fi

    : ${BEEFAULTS:=${BEE_SYSCONFDIR}/bee/beerc}
    if [ -r ${BEEFAULTS} ] ; then
	print_error \
            "WARNING: support for \${BEEFAULTS} (${BEEFAULTS}) is" \
            ' deprecated please move it to one of' \
	    ' ${XDG_CONFIG_DIRS}/bee/beerc'
	# just warn for now
    fi

    pathappend "${BEE_XDG_DATADIR}" XDG_DATA_DIRS
    pathappend "${BEE_SYSCONFDIR}"  XDG_CONFIG_DIRS

    for dir in ${XDG_CONFIG_HOME} ${XDG_CONFIG_DIRS} ; do
	xdgbeerc="${dir}/bee/beerc"
	print_info "checking config file ${xdgbeerc}"
	if [ -r ${xdgbeerc} ] ; then
            print_info "  -> loading ${xdgbeerc}"
	    . ${xdgbeerc}
	fi
    done

    # bee default values based on uid
    #   - root gets system defaults..
    #   - other get XDG_*_HOME defaults..
    if [ ${UID} -eq 0 ] ; then # ROOT
	: ${BEE_REPOSITORY_PREFIX=/usr/src/bee}
	: ${BEE_METADIR=${BEE_XDG_DATADIR}/bee}
    else # USER
	: ${BEE_REPOSITORY_PREFIX=${XDG_DATA_HOME}/beeroot}
	: ${BEE_METADIR=${XDG_DATA_HOME}/beemeta}
    fi

#    for dir in ${XDG_CONFIG_HOME} ${XDG_CONFIG_DIRS} ; do
#	xdgskiplist="${dir}/bee/skiplist"
#	print_info "checking skiplist file ${xdgskiplist}"
#	if [ -r "${xdgskiplist}" ] ; then
#	    : ${BEE_SKIPLIST=${xdgskiplist}}
#	fi
#    done

    : ${BEE_TMP_TMPDIR:=/tmp}
    : ${BEE_TMP_BUILDROOT:=${BEE_TMP_TMPDIR}/beeroot-${LOGNAME}}

    : ${BEE_REPOSITORY_BEEDIR:=${BEE_REPOSITORY_PREFIX}/bees}
    : ${BEE_REPOSITORY_PKGDIR:=${BEE_REPOSITORY_PREFIX}/pkgs}
    : ${BEE_REPOSITORY_BUILDARCHIVEDIR:=${BEE_REPOSITORY_PREFIX}/build-archives}

    print_info "  BEE_SKIPLIST           ${BEE_SKIPLIST}"
    print_info "  BEE_REPOSITORY_PREFIX  ${BEE_REPOSITORY_PREFIX}"
    print_info "  BEE_METADIR            ${BEE_METADIR}"
    print_info "  BEE_TMP_TMPDIR         ${BEE_TMP_TMPDIR}"
    print_info "  BEE_TMP_BUILDROOT      ${BEE_TMP_BUILDROOT}"

#    export BEE_SKIPLIST
    export BEE_REPOSITORY_PREFIX
    export BEE_REPOSITORY_BEEDIR
    export BEE_REPOSITORY_PKGDIR
    export BEE_REPOSITORY_BUILDARCHIVEDIR
    export BEE_METADIR
    export BEE_TMP_TMPDIR
    export BEE_TMP_BUILDROOT

    export BEE_SYSCONFDIR
    export BEE_DATADIR
    export BEE_LIBEXECDIR

    export XDG_CONFIG_HOME
    export XDG_CONFIG_DIRS
    export XDG_DATA_HOME
    export XDG_DATA_DIRS
}
###############################################################################
###############################################################################
###############################################################################

usage() {
    echo "usage: $0 <option> <arguments>"
    echo "possible options are:"
    echo "  init"
    echo "  install"
    echo "  remove"
    echo "  check"
    echo "  version"
}

init_config

case "$1" in
    init)
        shift
        bee-init $@
        ;;
    install)
        shift
        bee-install $@
        ;;
    remove)
        shift
        bee-remove $@
        ;;
    check)
        shift
        bee-check $@
        ;;
    version)
        shift
        beeversion $@
        ;;
    list)
        shift
        bee-list $@
        ;;
    query)
        shift
        bee-query $@
        ;;
    *)
        if [ -n "${1}" ] ; then
            echo "$1 is not a known option.."
        fi
        usage
        ;;
esac
