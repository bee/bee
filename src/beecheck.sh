#!/bin/bash
#
# bee-check - check consistancy of installed bee-pkgs
# Copyright (C) 2009-2010
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

BEESEP=beesep

###############################################################################
###############################################################################
###############################################################################

VERSION=0.1

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

###############################################################################
##
##
pkg_check_all() {

    if [ ! "${1}" ] ; then
        pkg_check
        return
    fi

    for pkg in ${@} ; do
        pkg_check ${pkg}
    done
}


###############################################################################
##
##
pkg_check_deps() {
    installed=$(bee-list --installed --exact ${1})
    
    if [ ! "${installed}" -a $OPT_F -gt 0 ] ; then
        installed=$(bee-list --installed "${1}")
    fi
   
    if [ "${installed}" ] ; then
        for i in ${installed} ; do
            do_check_deps "${i}"
        done
    fi
    exit 0
}

###############################################################################
##
##
pkg_check() {
    installed=$(bee-list --installed --exact ${1})
    
    if [ ! "${installed}" -a $OPT_F -gt 0 ] ; then
        installed=$(bee-list --installed "${1}")
    fi
    
    if [ "${installed}" ] ; then
        for i in ${installed} ; do
            do_check "${i}"
        done
        return 0
    fi
    
    installed=$(bee-list --installed "${1}")
    
    if [ "${installed}" ] ; then
        echo "packages matching '${1}':"
        for i in ${installed} ; do
            echo " [*] ${i}"
        done
    fi
    
    
}


###############################################################################
##
##
do_check_deps() {
    local pkg=${1}
    local filesfile=${BEE_METADIR}/${pkg}/FILES
    
    for line in $(cat ${filesfile}) ; do 
        local IFS=":"

        for ff in ${line} ; do
            # evil 8)...  don't hack me... hrhr
            eval $ff
        done

        # save and strip possible symbolic link destination..
        symlink=${file#*//}
        file=${file%%//*}
        
        if [ ! -f "${file}" -o -h ${file} ] ; then
             continue
        fi
        
#       echo ${file}
        
        readelf -d ${file} 2>/dev/null | egrep "(NEEDED|SONAME)"
        
    done \
      | sed -e 's,.*Shared library: \[\(.*\)\].*,needs \1,' \
            -e 's,.*Library soname: \[\(.*\)\].*,provides \1,' \
      | sort -u 
}

###############################################################################
##
##
do_check() {
    local pkg=${1}
    
    local filesfile=${BEE_METADIR}/${pkg}/FILES
    
    echo "checking ${pkg} .."
    
    for line in $(cat ${filesfile}) ; do 
        eval $(${BEESEP} ${line})

        # save and strip possible symbolic link destination..
        symlink=${file#*//}
        file=${file%%//*}

        if [ ! -e "${file}" ] && [ ! -h "${file}" ]; then
            echo "  [missing] <${md5}> ${file}"
            continue
        fi
        
        if [ "${md5}" = "link" ] ; then
            if [ ! -h "${file}" ] ; then
                echo "  [changed] <was symlink to ${symlink}> ${file}"
            else 
                sdest=$(readlink ${file})
                if [ "${sdest}"  != "${symlink}" ] ; then
                    echo "  [changed] <symlink desitination '${symlink}' != '${sdest}'> ${file}"
                fi
                #echo "  [ DEBUG ] <OK> ${file} -> ${symlink}"
            fi
            continue
        fi
        
        if [ "${md5}" = "directory" ] ; then
            if [ ! -d "${file}" ] ; then
                echo "  [changed] <was directory> ${file}"
            fi
            continue
        fi
        
        # regular file - check md5sum..
        
        md5now=$(md5sum ${file} | sed -e 's,^\([a-z0-9]*\).*$,\1,')

        if [ "${md5}" != "${md5now}" ] ; then
            echo "  [changed] <${md5} != ${md5now}> ${file}"
            continue
        fi
    done
    
}

###############################################################################
### get_installed_versions ###
##
## IN: full_packagename
##
## OUT: list of installed packages matching pkgname(full_packagename)
##
## DESCRIPTION: ...
##
get_installed_versions() {
    local pkg=${1}
    
    local list
    local pname=$(beeversion --pkgfullname ${pkg})
    
    if [ $? ] ; then
       echo "ERRRRRRRRR"
       exit 1
    fi
    
    
    
    for i in $(bee-list --installed "${pname}") ; do
        local installed=$(beeversion --pkgfullname ${i})
        if [ "${installed}" = "${pname}" ] ; then
            list="${list:+${list} }${i}"
        fi
    done
        
    echo "${list}"
}

##### usage ###################################################################
usage() {
    echo "bee-check v${VERSION} 2009-2010"
    echo ""
    echo "  by Tobias Dreyer and Marius Tolzmann <{dreyer,tolzmann}@molgen.mpg.de>"
    echo ""
    echo " Usage: $0 [action] [options] <pkg>"
    echo ""
    echo " action:"
    echo "   -d | --deps"
    echo "   -h | --help       display this help.. 8)"
    echo ""
    echo " options:"
    echo "   -f | --force      can be used to force check come what may"
    echo "   -v | --verbose    bee more verbose (can be used twice e.g. -vv)"
    echo
}

###############################################################################
##
##
options=$(getopt -n bee_check \
                 -o advfh \
                 --long verbose,all,force,help,deps \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

declare -i OPT_A=0
declare -i OPT_F=0

while true ; do
  case "$1" in
    -a|--all|-v|--verbose)
      shift;
      OPT_A=$OPT_A+1
      ;;
    -f|--force)
      shift;
      OPT_F=$OPT_F+1
      ;;
    -d|--deps)
      shift 2;
      pkg_check_deps ${@}
      exit 0
      ;;  
    -h|--help)
      usage
      exit 0
    ;;
    *)
      shift
      pkg_check_all ${@}
      exit 0;
      ;;
  esac
done
