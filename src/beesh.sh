#!/bin/bash
set -e

#save fd of stdout
exec 3<&1

#architecture
ARCH=$(arch)

# Version
VERSION=0.2

###############################################################################
###############################################################################
###############################################################################

#### create_meta() ############################################################

create_meta() {
    echo "PNF=${PNF}"       >> ${D}/META
    echo "PVF=${PVF}"       >> ${D}/META
    echo "PR=${PR}"         >> ${D}/META
    echo "PGRP=( ${PGRP} )" >> ${D}/META
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
    echo "PGRP=( uncategorized )"
    echo ""
    echo "IGNORE_DATAROOTDIR=1"
    echo "IGNORE_LOCALEDIR=1"
    echo "IGNORE_DOCDIR=1"
    echo ""
    echo "# PATCHURL=\"\""
    echo ""
    echo "# EXCLUDE=\"\""
    echo ""
    echo "# mee_configure() {"
    echo "#     bee_configure"
    echo "# }"
    echo ""
    echo "# mee_build() {"
    echo "#     bee_build"
    echo "# }"
    echo ""
    echo "# mee_install() {"
    echo "#     bee_install"
    echo "# }"

}

#### bee_init_builddir() ######################################################

bee_init_builddir() {
    if [ -d ${W} ] ; then 
        if [ "${OPT_CLEANUP}" = "yes" ] ; then
            echo "#BEE# cleaning work-dir ${W} .."
            rm -fr ${W}
        else
            echo "#BEE# error initializing build-dir ${W}"
            exit 1
        fi
    fi
    mkdir -p ${S}
    mkdir -p ${B}
    mkdir -p ${D}
}

#### bee_getsources() #########################################################
# fetch_one_file <url> [filename]

fetch_one_file() {
    url=$1
    file=${2:-$(basename ${url})}
    
    if [ ${url:0:4} == "file" ] ; then
        echo "#BEE# copying file ${url}"
        u=$(echo $url | sed -e 's,^file://,,' - )
        cp -v ${u} ${F}/${file}
    else
        if [ ${url:0:5} == "https" ] ; then
            nocheck="--no-check-certificate"
        else
            nocheck=""
        fi

        if [ ! -s ${F}/${file} ] ; then
            rm -vf ${F}/${file}
        fi
        echo "#BEE# fetching $url"
        wget \
            ${nocheck} \
            --output-document="${F}/${file}" \
            --no-clobber \
            "${url}" || true

        ls -ld "${F}/${file}"
    fi

    A=(${A[@]} ${file})
}


# fetch_one_patch <url>  [filename]
# fetch_one_patch <path>
fetch_one_patch() {
    url=$1
    file=${2:-$(basename ${url})}
    
    if [ ${url:0:4} == "file" ] ; then 
        echo "#BEE# copying patch ${url}"
        u=$(echo $url | sed -e 's,^file://,,' - )
        cp -v ${u} ${F}/${file}
    else 
        if [ ${url:0:5} == "https" ] ; then
            nocheck="--no-check-certificate"
        else
            nocheck=""
        fi
        echo "#BEE# fetching patch ${url}"
        wget \
            ${nocheck} \
            --output-document="${F}/${file}" \
            --no-clobber \
            "${url}" || true

        ls -ld "${F}/${file}"
    fi
    
    PA="${PA:+${PA} }${file}"
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

    if [ -z ${SRCURL} ] ; then 
        unset SRCURL
    fi

    for s in "${SRCURL[@]}" ; do
        fetch_one_file ${s}
    done

    if [ -z ${PATCHURL} ] ; then 
        unset PATCHURL
    fi

    if [ -z ${PATCHES} ] ; then
        unset PATCHES
    fi

    if [ -z "${PATCHURL}" ] && [ -n "${PATCHES}" ] ; then
        echo '#BEE# warning .. you are using obsolete variable $PATCHES .. please use $PATCHURL instead'
        PATCHURL=(${PATCHES[@]})
    fi

    for p in "${PATCHURL[@]}" ; do
        fetch_one_patch ${p}
    done
}

#### bee_unpack() #############################################################

bee_unpack() {
    
    if [ -z ${A[0]} ] ; then return ; fi
    
    s=${A[0]}
    echo "#BEE# unpacking source ${s} .."
    tar xof ${F}/${s} --strip-components 1 -C ${S}
    
    unset A[0]
    
    for s in ${A[@]} ; do
        echo "#BEE# unpacking source ${s} .."
        tar xof ${F}/${s} -C ${S}
    done
}

#### bee_patch() ##############################################################

bee_patch() {
    for p in ${PA} ; do
        echo "#BEE# applying patch ${p} .."
        patch -Np1 -i ${F}/${p}
    done
}

#### bee_configure() ##########################################################

bee_configure() {
    if [ "${BEE_CONFIGURE}" == "none" ] ; then
        return 0
    fi
    
    echo "#BEE# configuring .."
    echo "#BEE#   => ${S}/configure ${DEFCONFIG} $@"
    ${OPT_SILENT:+eval exec 1>/dev/null}
    ${S}/configure ${DEFCONFIG} $@
    ${OPT_SILENT:+eval exec 1>&3}
}

#### bee_build() ##############################################################

bee_build() {
    echo "#BEE# make $@"
    ${OPT_SILENT:+eval exec 1>/dev/null}
    make $@
    ${OPT_SILENT:+eval exec 1>&3}
}

#### bee_install() ############################################################

bee_install() {
    echo "#BEE# make install $@"
    ${OPT_SILENT:+eval exec 1>/dev/null}
    make install DESTDIR=${D}
    ${OPT_SILENT:+eval exec 1>&3}
}

#### bee_pkg_pack() ###########################################################

# $EXCLUDE is read from .bee file
# $BEESKIPLIST is found in $BEEFAULTS
bee_pkg_pack() {
    for e in ${EXCLUDE} ; do
        exargs="${exargs} --exclude=${e}";
    done

    beefind.pl ${BEESKIPLIST:+--excludelist=${BEESKIPLIST}} \
               --exclude='^/FILES$' ${exargs} \
               --cutroot=${D} ${D} > ${D}/FILES 2>/dev/null

    DUMP=${TEMPDIR}/bee.$$.dump

    beefind.pl --dump ${D}/FILES | sed -e "s,^,${D}," - > ${DUMP}

    cp ${BEE} ${D}/BUILD

    create_meta
    
    if [ -n "${PA}" ] ; then
        mkdir -pv ${D}/PATCHES
    fi
    for p in ${PA} ; do
        cp ${F}/${p} ${D}/PATCHES/${p}
    done

    if [ ! -d ${BEEPKGSTORE} ] ; then
        mkdir -pv ${BEEPKGSTORE}
    fi 

    echo "#BEE# ${BEEPKGSTORE}/${PF}.${PARCH}.bee.tar.bz2 contains .."

    tar cjvvf ${BEEPKGSTORE}/${PF}.${PARCH}.bee.tar.bz2 \
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
        ${D}/{FILES,BUILD,META} ${PA:+${D}/PATCHES} ${PA:+${D}/PATCHES/*}

    rm ${DUMP}

    if [ ! -d ${BEESTORE} ] ; then
        mkdir -pv ${BEESTORE}
    fi 

    cp -v ${BEE} ${BEESTORE}
}


#### mee_*() ##################################################################

mee_getsources() { bee_getsources ; }
mee_unpack()     { bee_unpack;      }
mee_patch()      { bee_patch;       }

mee_configure()  { bee_configure;   }
mee_build()      { bee_build;       }
mee_install()    { bee_install ;    }

###############################################################################
###############################################################################
###############################################################################

OPTIONS=$(getopt -o hifcs --long help,install,force-install,cleanup,silent-build -n bee-option-parser -- "$@")

if [ $? != 0 ] ; then 
    echo "Terminating..." >&2
    exit 1
fi

eval set -- "${OPTIONS}"

OPT_INSTALL="no" 
OPT_CLEANUP="no"

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
        --) 
	    shift
	    break
	    ;;
        *) 
	    echo "Internal error!"
	    exit 1
	    ;;
    esac
done

###############################################################################
###############################################################################
###############################################################################

BEE=$1
if [ ! ${BEE:0:1} = "/" ] ; then
    BEE=${PWD}/$BEE
fi

PF=$(basename $BEE .bee)

#parse pkg name
# PF  = glibc-2.10.1_rc5-1 - full package name
# PNF =                    - package full name
# PN  = glibc              - package name
# PNE =                    - package name extension
# PVF = 2.10.1_rc5         - version unterversion
# PV  = 2.10.1             - version
# PS  = rc5                - unterversion
# PR  = 1                  - revision
# PVR = 2.10.1_rc5-1       - version-revision (full version)
# P   = glibc-2.10.1_rc5   - package name (ohne revision)
PNF=$(echo ${PF} | sed -e 's,^\(.*\)-\(.*\)-\(.*\)$,\1,' - )
PN=$(echo ${PNF} | sed -e 's,\(.*\)\(_.*\),\1,' - )
PNE=$(echo ${PNF} | sed -e '/_/!d' - | sed -e 's,\(.*\)_\(.*\),\2,' - )

PVF=$(echo ${PF} | sed -e 's,^\(.*\)-\(.*\)-\(.*\)$,\2,' - )
PV=$(echo ${PVF} | sed -e 's,\(.*\)\(_.*\),\1,' - )
PS=$(echo ${PVF} | sed -e '/_/!d' - | sed -e 's,\(.*\)_\(.*\),\2,' - )

PR=$(echo ${PF} | sed -e 's,^\(.*\)-\(.*\)-\(.*\)$,\3,' - )

PVR=${PVF}-${PR}
P=${PNF}-${PVF}

: ${BEEFAULTS=/etc/bee/beerc}

if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${BEEROOT=/tmp/beeroot}

#pkg root
R=${BEEROOT}/${PNF}

#pkg files .tar .patch ..
F=${R}/files

#workdir for current build
W=${R}/${PF}

S=${W}/source
B=${W}/build
D=${W}/image

###############################################################################

. ${BEE}

###############################################################################

: ${BEESKIPLIST:=/etc/bee/skiplist}

#root dir for bee
: ${BEESTORE:=/usr/src/bee/bees}
: ${BEEPKGSTORE:=/usr/src/bee/pkgs}

: ${TEMPDIR:=/tmp}

: ${PARCH:=${ARCH}}

#define default directories

: ${PREFIX:=/usr}
: ${EPREFIX:=${PREFIX}}
: ${BINDIR:=${EPREFIX}/bin}

: ${SBINDIR:=${EPREFIX}/sbin}
: ${LIBEXECDIR:=${EPREFIX}/lib/${PNF}}
: ${SYSCONFDIR:=/etc}

: ${LOCALSTATEDIR:=/var}
: ${SHAREDSTATEDIR:=${LOCALSTATEDIR}}
: ${LIBDIR:=${EPREFIX}/lib}
: ${INCLUDEDIR:=${PREFIX}/include}
: ${DATAROOTDIR:=${PREFIX}/share}
: ${DATADIR:=${DATAROOTDIR}}
: ${INFODIR:=${DATAROOTDIR}/info}
: ${MANDIR:=${DATAROOTDIR}/man}
: ${DOCDIR:=${DATAROOTDIR}/doc/gtkhtml}
: ${LOCALEDIR:=${DATAROOTDIR}/locale}


# check IGNORE_DATAROOTDIR for compatibility with old bee-files
if [ ${IGNORE_DATAROOTDIR} ] ; then
    echo "IGNORE_DATAROOTDIR is deprecated! pleade use BEE_CONFIGURE='compat' instead."
    BEE_CONFIGURE='compat'
fi

if [ "${BEE_CONFIGURE}" == "compat" ] ; then
    unset DATAROOTDIR
    unset LOCALEDIR
    unset DOCDIR
fi

#define default configure
: ${DEFCONFIG:="\
--prefix=${PREFIX} \
--exec-prefix=${EPREFIX} \
--bindir=${BINDIR} \
--sbindir=${SBINDIR} \
--libexecdir=${LIBEXECDIR} \
--sysconfdir=${SYSCONFDIR} \
--sharedstatedir=${SHAREDSTATEDIR} \
--localstatedir=${LOCALSTATEDIR} \
--libdir=${LIBDIR} \
--includedir=${INCLUDEDIR} \
${DATAROOTDIR:+--datarootdir=${DATAROOTDIR}} \
--datadir=${DATADIR} \
--infodir=${INFODIR} \
${LOCALEDIR:+--localedir=${LOCALEDIR}} \
--mandir=${MANDIR} \
${DOCDIR:+--docdir=${DOCDIR}} \
"}

bee_init_builddir
mee_getsources
mee_unpack
cd ${S}
mee_patch
cd ${B}
mee_configure
mee_build
mee_install
cd ${D}
bee_pkg_pack

if [ $OPT_INSTALL = "yes" ] ; then
    echo "installing ${PF}.${PARCH}.."
    bee install ${OPT_FORCE:+-f} ${BEEPKGSTORE}/${PF}.${PARCH}.bee.tar.bz2
fi
