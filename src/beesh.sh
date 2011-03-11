#!/bin/bash
set -e

#save fd of stdout
exec 3<&1

#architecture
ARCH=$(arch)

# Version
VERSION=0.4

BEE_SYSCONFDIR=/etc/bee
BEE_DATADIR=/usr/share

: ${DOTBEERC:=${HOME}/.beerc}
if [ -e ${DOTBEERC} ] ; then
    . ${DOTBEERC}
fi

: ${BEEFAULTS:=${BEE_SYSCONFDIR}/beerc}
if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${BEE_METADIR=${BEE_DATADIR}/bee}
: ${BEE_REPOSITORY_PREFIX=/usr/src/bee}

: ${BEE_TMP_TMPDIR:=/tmp}
: ${BEE_TMP_BUILDROOT:=${BEE_TMP_TMPDIR}/beeroot-${LOGNAME}}

: ${BEE_SKIPLIST=${BEE_SYSCONFDIR}/skiplist}

# copy file.bee to ${BEE_REPOSITORY_BEEDIR} after successfull build
: ${BEE_REPOSITORY_BEEDIR:=${BEE_REPOSITORY_PREFIX}/bees}

# directory where (new) bee-pkgs are stored
: ${BEE_REPOSITORY_PKGDIR:=${BEE_REPOSITORY_PREFIX}/pkgs}

# directory where copies of the source+build directories are stored
: ${BEE_REPOSITORY_BUILDARCHIVEDIR:=${BEE_REPOSITORY_PREFIX}/build-archives}

COLOR_NORMAL="\\033[0;39m\\033[0;22m"
COLOR_GREEN="\\033[0;32m"
COLOR_YELLOW="\\033[0;33m"
COLOR_RED="\\033[0;31m"
COLOR_CYAN="\\033[0;36m"
COLOR_BLUE="\\033[0;34m"
COLOR_PURPLE="\\033[0;35m"

COLOR_BRACKET=${COLOR_PURPLE}
COLOR_BRCONTENT=${COLOR_YELLOW}
COLOR_INFO=${COLOR_GREEN}

print_info() {
    echo -e ${COLOR_BRACKET}[${COLOR_BRCONTENT}BEE${COLOR_BRACKET}] ${COLOR_INFO}${@}${COLOR_NORMAL}
}

log_enter() {
    print_info ">>>> entering ${@} .."
}

log_leave() {
    print_info "<<<< leaving ${@} .."
}

start_cmd() {
    print_info "${COLOR_CYAN}${@}"
    ${OPT_SILENT:+eval exec 1>/dev/null}
    "${@}"
    ${OPT_SILENT:+eval exec 1>&3}
}

###############################################################################
###############################################################################
###############################################################################

#### create_meta() ############################################################

create_meta() {
    echo >>${D}/META "BEEMETAFORMAT=1"
    echo >>${D}/META "BEEPKG='${PKGALLPKG}'"
    echo >>${D}/META "PGRP=( ${PGRP[@]} )"
}

#### show_help() ##############################################################

show_help() {
    echo " beesh v${VERSION} 2009-2010"
    echo ""
    echo "  by Tobias Dreyer and Marius Tolzmann <{dreyer,tolzmann}@molgen.mpg.de>"
    echo ""
    echo " Usage: $0 [options] <pkg>.bee"
    echo ""
    echo " Options:"
    echo "   -c | --cleanup       .. may be used to clean up <pkg>-related"
    echo "                           directory tree before build process is started"
    echo "   -i | --install       .. after build process is successful and <pkg> is"
    echo "                           built, <pkg>.tgz is install by bee_install"
    echo "   -f | --force-install .. same as -i; bee_install is invoked with --force"
    echo "   -h | --help          .. display this help.. 8)"
    echo ""
    echo ""
    echo " Beefile example.."
    echo ""
    echo "#!/bin/env beesh"
    echo "SRCURL=\"ftp://ftp.gmplib.org/pub/gmp-4.3.1/gmp-4.3.1.tar.bz2\""
    echo ""
    echo "# PATCHURL[0]="
    echo ""
    echo "PGRP=( uncategorized )"
    echo ""
    echo "BEE_CONFIGURE=\"compat\""
    echo ""
    echo "# EXCLUDE=\"\""
    echo ""
    echo "BEEPASSES=1"
    echo ""
    echo "mee_configure() {"
    echo "    bee_configure"
    echo "}"
    echo "# mee_configure2() {"
    echo "#     bee_configure"
    echo "# }"
    echo ""
    echo "mee_build() {"
    echo "    bee_build"
    echo "}"
    echo "# mee_build2() {"
    echo "#     bee_build"
    echo "# }"
    echo ""
    echo "mee_install() {"
    echo "    bee_install"
    echo "}"
    echo "# mee_install2() {"
    echo "#     bee_install"
    echo "# }"

}

#### bee_init_builddir() ######################################################

bee_init_builddir() {
    
    print_info "==> initializing build environment .."
    
    if [ -d "${W}" ] ; then 
        if [ "${OPT_CLEANUP}" = "yes" ] ; then
            print_info " -> cleaning work dir ${W} .."
            rm -fr ${W}
        else
            print_info "error initializing build-dir ${W}"
            exit 1
        fi
    fi
    
    print_info " -> creating source dir ${S}"
    
    mkdir -p ${S}
    
    
    if [ "${B}" == "${S}" ] ; then
        B=${BEEWORKDIR}/build
        print_info " -> B=S linking build dir ${B} to source dir"
        ln -s source ${B}
    else
        print_info " -> creating build dir ${B}"
        mkdir -p ${B}
    fi
    
    print_info " -> creating image dir ${D}"
    mkdir -p ${D}
    echo
}

#### bee_getsources() #########################################################
# fetch_one_file <url> [filename]

fetch_one_file() {
    url=$1
    file=${2:-$(basename ${url})}
    
    if [ "${url:0:8}" = "file:///" ] ; then
        url=${url:7}
    fi
    
    if [ "${url:0:1}" = "/" ] ; then
        print_info "copying file ${url}"
        cp -v "${url}" "${F}/${file}"
    else
        if [ ${url:0:5} == "https" ] ; then
            nocheck="--no-check-certificate"
        else
            nocheck=""
        fi
        
        if [ ! -s "${F}/${file}" ] ; then
            rm -vf ${F}/${file}
        fi
        
        print_info "fetching $url"
        wget \
            ${nocheck} \
            --no-verbose \
            --output-document="${F}/${file}" \
            --no-clobber \
            "${url}" || true

        ls -ld "${F}/${file}"
    fi

    bee_FETCHED_FILE="${F}/${file}"
    bee_FETCHED_FILES=( ${bee_FETCHED_FILES[@]} "${F}/${file}" )
}

fetch_one_archive() {
    fetch_one_file $@
    
    bee_SOURCEFILES=( ${bee_SOURCEFILES[@]} ${bee_FETCHED_FILE} )
}

fetch_one_patch() {
    fetch_one_file $@
    
    bee_PATCHFILES=( ${bee_PATCHFILES[@]} ${bee_FETCHED_FILE} )
}

bee_getsrcurl() {
    local -a archives=( "${@}" )
    
    for a in "${archives[@]}" ; do
        fetch_one_archive ${a}
    done
}

bee_getpatchurl() {
    local -a patches=( "${@}" )
    
    for p in "${patches[@]}" ; do
        fetch_one_patch ${p}
    done
}

# bee_getsources
#    SRCURL[] = "<url> [filename]"
# e.g.:
#    SRCURL=("<url> [filename]" "<url> [filename]")
#    SRCURL="<url> [filename]"
#    SRCURL[0]="<url> [filename]"
#    SRCURL[1]="<url> [filename]"

bee_getsources() {
    mkdir -p "${F}"

    if [ -z "${SRCURL}" ] ; then 
        unset SRCURL
    fi

    bee_run getsrcurl "${SRCURL[@]}"

    if [ -z "${PATCHURL}" ] ; then 
        unset PATCHURL
    fi

    if [ -z "{PATCHES}" ] ; then
        unset PATCHES
    fi

    if [ -z "${PATCHURL}" ] && [ -n "${PATCHES}" ] ; then
        print_info 'warning .. you are using deprecated variable ${PATCHES} .. please use ${PATCHURL} instead'
        PATCHURL=( "${PATCHES[@]}" )
    fi

    bee_run getpatchurl "${PATCHURL[@]}"
}

#### bee_extract() #############################################################

bee_extract() {
    local bee_S
    bee_S=( $@ )
    
    log_enter "bee_extract()"

    if is_func mee_unpack ; then
        print_info "#BEE-WARNING# function 'mee_unpack()' is deprecated .. use 'mee_extract()' instead .." >&2
        bee_run unpack "${@}"
        log_leave "bee_extract()"
        return
    fi
    
    if [ -z "${bee_S[0]}" ] ; then 
        log_leave "bee_extract()"
        return 
    fi
    
    s=${bee_S[0]}
    print_info " -> extracting main source ${s} .."
    tar xof ${s} --strip-components 1 -C ${S}
    
    unset bee_S[0]
    
    for s in ${bee_S[@]} ; do
        print_info " -> extracting additional source ${s} .."
        tar xof ${s} -C ${S}
    done
    
    print_info " -> all sources extracted to: ${S} .."
    log_leave "bee_extract()"
}

#### bee_patch() ##############################################################

bee_patch() {
    local bee_P
    bee_P=( $@ )
    
    log_enter "bee_patch()"
    
    if [ ${#bee_P[@]} == 0 ] ; then
        bee_P=( ${bee_PATCHFILES[@]} )
    fi
    
    for p in ${bee_P[@]} ; do
        print_info "applying patch ${p} .."
        patch -Np1 -i ${p}
    done
    log_leave "bee_patch()"
}

#### bee_configure() ##########################################################

bee_configure() {

    log_enter "bee_configure()"
    
    if [ "${BEE_CONFIGURE}" == "none" ] ; then
        print_info " -- skipping bee_configure .. (BEE_CONFIGURE=none)"
        log_leave "bee_configure()"
        return 0
    fi
    
    start_cmd ${S}/configure ${DEFCONFIG} "$@"
    
    log_leave "bee_configure()"
}

#### bee_build() ##############################################################

bee_build() {
    log_enter "bee_build()"
    
    start_cmd make $@
    
    log_leave "bee_build()"
}

#### bee_install() ############################################################

bee_install() {
    log_enter "bee_install()"
    
    print_info " -> installing files to ${D}"
    start_cmd make install DESTDIR=${D} $@
    
    log_leave "bee_install()"
}

#### bee_pkg_pack() ###########################################################

# $EXCLUDE is read from .bee file
# $BEE_SKIPLIST is found in $BEEFAULTS
bee_pkg_pack() {
    log_enter "bee_pkg_pack()"
    
    for e in ${EXCLUDE} ; do
        exargs="${exargs} --exclude=${e}";
    done

    beefind.pl ${BEE_SKIPLIST:+--excludelist=${BEE_SKIPLIST}} \
               --exclude='^/FILES$' ${exargs} \
               --cutroot=${D} ${D} > ${D}/FILES 2>/dev/null

    DUMP=${BEE_TMP_TMPDIR}/bee.$$.dump

    beefind.pl --dump ${D}/FILES | sed -e "s,^,${D}," - > ${DUMP}

    cp ${BEE} ${D}/BUILD

    create_meta
    
    if [ -n "${bee_PATCHFILES[0]}" ] ; then
        mkdir -pv ${D}/PATCHES
    fi
    for p in ${bee_PATCHFILES[@]} ; do
        cp ${p} ${D}/PATCHES
    done

    if [ ! -d "${BEE_REPOSITORY_PKGDIR}" ] ; then
        mkdir -pv ${BEE_REPOSITORY_PKGDIR}
    fi 
    
    pkgname=${PKGALLPKG}.bee.tar.bz2
    pkgfile=${BEE_REPOSITORY_PKGDIR}/${pkgname}
    
    print_info " -> creating package ${pkgname} .."
    print_info "${COLOR_CYAN}${pkgfile}"

    tar cjvvf ${pkgfile} \
        -T ${DUMP} \
        --transform="s,${D},," \
        --show-transformed-names \
        --sparse \
        --absolute-names \
        --no-recursion \
        --transform="s,^/FILES$,FILES," \
        --transform="s,^/BUILD$,BUILD," \
        --transform="s,^/META$,META," \
        --transform="s,^/PATCHES,PATCHES," \
        ${D}/{FILES,BUILD,META} \
        ${bee_PATCHFILES:+${D}/PATCHES} \
        ${bee_PATCHFILES:+${D}/PATCHES/*}

    rm ${DUMP}
    
    beefilename=$(basename ${BEE})
    beefiledest=${BEE_REPOSITORY_BEEDIR}/${beefilename}
    
    print_info "-> saving bee-file ${beefilename} .."
    print_info "${COLOR_CYAN}${beefiledest}"

    if [ ! -d "${BEE_REPOSITORY_BEEDIR}" ] ; then
        mkdir -pv ${BEE_REPOSITORY_BEEDIR}
    fi 

    cp -v ${BEE} ${BEE_REPOSITORY_BEEDIR}
    
    log_leave "bee_pkg_pack()"
}


bee_archivebuild() {
    [ "${OPT_ARCHIVE_BUILD}" != "yes" ] && return

    log_enter "bee_archivebuild()"
    
    if [ ! -d "${BEE_REPOSITORY_BUILDARCHIVEDIR}" ] ; then
        mkdir -pv ${BEE_REPOSITORY_BUILDARCHIVEDIR}
    fi
    
    archive="${BEE_REPOSITORY_BUILDARCHIVEDIR}/${PKGALLPKG}.beebuild.tar.bz2"
    
    print_info " -> saving build environment.."
    print_info "${COLOR_CYAN}${archive}"
    
    tar -cjf ${archive} \
        --show-transformed-names \
        --sparse \
        --absolute-names \
        ${S} ${B} \
        ${bee_FETCHED_FILES[@]} \
        ${BEE_REPOSITORY_BEEDIR}/$(basename ${BEE}) \
        --transform="s,^${BEEWORKDIR},${PKGALLPKG}," \
        --transform="s,^${F},${PKGALLPKG}/files," \
        --transform="s,^${BEE_REPOSITORY_BEEDIR},${PKGALLPKG}/files,"

    log_leave "bee_archivebuild()"
}

###############################################################################
###############################################################################
###############################################################################

dump_variables() {
    for i in P{,N{,F,E},F,V{,E,F,R},S,R} ${!PKG*} ${!BEE*} ${!DEF*} ${!OPT*} ${!DOT*} R F W S B D ; do
        eval echo "${i}=\${${i}}"
    done
}

is_func() {
    [ "$(type -t ${1})" == "function" ]
    return $?
}

bee_run() {
    action=${1}
    shift
    
    if is_func "mee_${action}${bee_PASS}" ; then
        log_enter "mee_${action}${bee_PASS}()"
        mee_${action}${bee_PASS} "${@}"
        log_leave "mee_${action}${bee_PASS}()"
    elif is_func "mee_${action}" ; then
        log_enter "mee_${action}()"
        mee_${action} "${@}"
        log_leave "mee_${action}()"
    else
        bee_${action} "${@}"
    fi
}

###############################################################################

OPTIONS=$(getopt -n bee-option-parser \
                 -o hifcs \
                 --long help,install,force-install,cleanup,silent-build,debug: \
                 --long archive-build,no-archive-build \
                 -- "$@")

if [ $? != 0 ] ; then 
    print_info "Terminating..." >&2
    exit 1
fi

eval set -- "${OPTIONS}"

: ${OPT_INSTALL:="no"}
: ${OPT_CLEANUP:="no"}
: ${OPT_ARCHIVE_BUILD:="yes"}

while true ; do
    case "$1" in
        -h|--help)
            show_help
            exit 0
            ;;
        -i|--install)
            OPT_INSTALL="yes"
            shift
            ;;
        -f|--force-install)
            OPT_INSTALL="yes"
            OPT_FORCE="yes"
            shift
            ;;
        -c|--cleanup) 
            OPT_CLEANUP="yes"
            shift 
            ;;
        -s|--silent-build)
            OPT_SILENT="yes"
            shift
            ;;
        --no-archive-build)
            OPT_ARCHIVE_BUILD="no"
            shift
            ;;
        --archive-build)
            OPT_ARCHIVE_BUILD="yes"
            shift
            ;;
        --debug)
            DEBUG=$2
            shift 2
            ;;
        --) 
            shift
            break
            ;;
        *) 
            print_info "Internal error!"
            exit 1
            ;;
    esac
done

###############################################################################
###############################################################################
###############################################################################

BEE=$1
if [ "${BEE:0:1}" != "/" ] ; then
    BEE=${PWD}/$BEE
fi

### define pkg variables
eval $(beeversion ${BEE})

PN=${PKGNAME}
PV=( ${PKGVERSION[@]} )
PS=${PKGEXTRAVERSION}
PR=${PKGREVISION}

P=${PKGFULLNAME}-${PKGFULLVERSION}

BEEPKGROOT="${BEE_TMP_BUILDROOT}/${PKGNAME}"
BEEWORKDIR="${BEEPKGROOT}/${PKGFULLPKG}"

R=${BEEPKGROOT}
W=${BEEWORKDIR}

F=${BEEPKGROOT}/files
S=${BEEWORKDIR}/source
B=${BEEWORKDIR}/build
D=${BEEWORKDIR}/image

###############################################################################

# clear PKGALLPKG since we can't trust PKGARCH in this state
PKGALLPKG=

# source file.bee 
. ${BEE}

# now set PKGARCH if set or changed by user via ARCH=.. and not given via file.arch.bee
: ${PKGARCH:=${ARCH}}

# since PKGARCH is now known reconstruct PKGALLPKG
: ${PKGALLPKG:=${PKGFULLPKG}.${PKGARCH}}

###############################################################################

# define defaults for bee_configure
: ${PREFIX:='/usr'}
: ${EPREFIX:='${PREFIX}'}
: ${BINDIR:='${EPREFIX}/bin'}

: ${SBINDIR:='${EPREFIX}/sbin'}
: ${LIBEXECDIR:='${EPREFIX}/lib/${PKGNAME}'}
: ${SYSCONFDIR:='/etc'}

: ${LOCALSTATEDIR:='/var'}
: ${SHAREDSTATEDIR:='${LOCALSTATEDIR}'}
: ${LIBDIR:='${EPREFIX}/lib'}
: ${INCLUDEDIR:='${PREFIX}/include'}
: ${DATAROOTDIR:='${PREFIX}/share'}
: ${DATADIR:='${DATAROOTDIR}'}
: ${INFODIR:='${DATAROOTDIR}/info'}
: ${MANDIR:='${DATAROOTDIR}/man'}
: ${DOCDIR:='${DATAROOTDIR}/doc/gtkhtml'}
: ${LOCALEDIR:='${DATAROOTDIR}/locale'}

# check IGNORE_DATAROOTDIR for compatibility with old bee-files
if [ ${IGNORE_DATAROOTDIR} ] ; then
    echo "#BEE-WARNING# IGNORE_DATAROOTDIR is deprecated! pleade use BEE_CONFIGURE='compat' instead." >&2
    BEE_CONFIGURE='compat'
fi

### create default configure line
: ${DEFCONFIG:='
--prefix=${PREFIX}
--exec-prefix=${EPREFIX}
--bindir=${BINDIR}
--sbindir=${SBINDIR}
--libexecdir=${LIBEXECDIR}
--sysconfdir=${SYSCONFDIR}
--sharedstatedir=${SHAREDSTATEDIR}
--localstatedir=${LOCALSTATEDIR}
--libdir=${LIBDIR}
--includedir=${INCLUDEDIR}
${DATAROOTDIR:+--datarootdir=${DATAROOTDIR}}
--datadir=${DATADIR}
--infodir=${INFODIR}
${LOCALEDIR:+--localedir=${LOCALEDIR}}
--mandir=${MANDIR}
${DOCDIR:+--docdir=${DOCDIR}}'}

eval PREFIX=${PREFIX}
eval EPREFIX=${EPREFIX}
eval BINDIR=${BINDIR}
eval SBINDIR=${SBINDIR}
eval LIBDIR=${LIBDIR}
eval SYSCONFDIR=${SYSCONFDIR}
eval LOCALSTATEDIR=${LOCALSTATEDIR}
eval SHAREDSTATEDIR=${SHAREDSTATEDIR}
eval LIBEXECDIR=${LIBEXECDIR}
eval INCLUDEDIR=${INCLUDEDIR}
eval DATAROOTDIR=${DATAROOTDIR}
eval DATADIR=${DATADIR}
eval INFODIR=${INFODIR}
eval LOCALEDIR=${LOCALEDIR}
eval MANDIR=${MANDIR}
eval DOCDIR=${DOCDIR}

if [ "${BEE_CONFIGURE}" = "compat" ] ; then
    unset DATAROOTDIR
    unset LOCALEDIR
    unset DOCDIR
fi

eval DEFCONFIG=\"${DEFCONFIG}\"

: ${BEEPASSES:=1}
declare -i bee_PASS=1

echo -e "${COLOR_CYAN}BEE - mariux package management "
echo -e "  by Marius Tolzmann und Tobias Dreyer {tolzmann,dreyer}@molgen.mpg.de"
echo -e "${COLOR_NORMAL}"

# in ${PWD}
bee_init_builddir

print_info "==> building ${PKGALLPKG} ..\n"

while [ ${bee_PASS} -le ${BEEPASSES} ] ; do
    print_info "running PASS ${bee_PASS} of ${BEEPASSES} .."
    echo
    bee_run getsources
    echo
    bee_run extract ${bee_SOURCEFILES[@]}
    echo
    print_info "changing dir to source: ${S}"
    cd ${S}
    echo
    bee_run patch ${bee_PATCHFILES[@]}
    echo
    print_info "changing dir to build: ${B}"
    cd ${B}
    echo
    bee_run configure
    echo
    bee_run build
    echo
    bee_run install
    echo
    bee_PASS=${bee_PASS}+1
done
cd ${D}

bee_pkg_pack

cd ${BEEWORKDIR}
bee_archivebuild

echo
print_info "==================================================================="
print_info "build summary:"
print_info ""
print_info "source directory: ${COLOR_NORMAL}${S}"
print_info " build directory: ${COLOR_NORMAL}${B}"
print_info " image directory: ${COLOR_NORMAL}${D}"
print_info ""
print_info "     bee-file: ${COLOR_NORMAL}${beefiledest}"
print_info "     pkg-file: ${COLOR_NORMAL}${pkgfile}"
print_info "build-archive: ${COLOR_NORMAL}${archive}"
print_info "==================================================================="
echo


if [ "${OPT_INSTALL}" = "yes" ] ; then
    print_info "installing ${PKGALLPKG} .."
    bee install ${OPT_FORCE:+-f} ${BEE_REPOSITORY_PKGDIR}/${PKGALLPKG}.bee.tar.bz2
fi
