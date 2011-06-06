#!/bin/bash
#
# bee-install - install a bee-pkg
# Copyright (C) 2009-2010 
#       Tobias Dreyer <dreyer@molgen.mpg.de>
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
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

if [ -z ${BEE_VERSION} ] ; then
    echo >&2 "BEE-ERROR: please call $0 from bee .."
    exit 1
fi

VERSION=${BEE_VERSION}

debug_msg() {
    if [ "${DEBUG}" != "yes" ] ; then
        return
    fi
    
    echo "#DEBUG# ${@}" >&2
}

start_cmd() {
    debug_msg "executing '${@}'"
    ${@}
}

##### usage ###################################################################
usage() {
    echo "bee-install v${VERSION} 2009-2010"
    echo ""
    echo "  by Tobias Dreyer and Marius Tolzmann <{dreyer,tolzmann}@molgen.mpg.de>"
    echo ""
    echo " Usage: $0 [action] [options] <pkg>"
    echo ""
    echo " action:"
    echo "   -i | --install    install package (default action)"
    echo "   -u | --upgrade    install package"
    echo "   -h | --help       display this help.. 8)"
    echo ""
    echo " options:"
    echo "   -f | --force      can be used to force installation come what may"
    echo "   -v | --verbose    bee more verbose (can be used twice e.g. -vv)"
    echo
}

###############################################################################
##
##
pkg_install_all() {
    for pkg in ${@} ; do
        pkg_install ${pkg}
    done
}

begins_with() {
    string=${1}
    substring=${2}
    
    length=${#substring}
    
    if [ ${#substring} -gt ${#string} ] ; then
        return 1
    fi
    
    if [ "${string:0:${length}}" = "${substring}" ] ; then
        return 0
    fi
    
    return 1
}

###############################################################################
### pkg_install ###
##
## IN: search-pattern
##
## OUT: ...
##
## DESCRIPTION: ...
##
pkg_install() {
    search=${1}
    
    # install specific package
    if [ -f "${search}" ] ; then
         do_install "${search}"
    fi
    
    # if search is relative or absolute path to file -> exit
    if [ "${search:0:2}" = "./" -o "$(dirname ${search})" != "." ] ; then
        echo "${search}: no such File or Directory"
        exit 1
    fi
    
    # install package from repository
    if [ -f "${BEE_REPOSITORY_PKGDIR}/${search}" ] ; then
         do_install "${BEE_REPOSITORY_PKGDIR}/${search}"
    fi
    
    for e in "" ".$(arch)" ".noarch" ".any" ; do
        if [ -f "${BEE_REPOSITORY_PKGDIR}/${search}${e}.bee.tar.bz2" ] ; then
            do_install "${BEE_REPOSITORY_PKGDIR}/${search}${e}.bee.tar.bz2"
        elif [ -f "${BEE_REPOSITORY_PKGDIR}/${search}${e}.iee.tar.bz2" ] ; then
            do_install "${BEE_REPOSITORY_PKGDIR}/${search}${e}.iee.tar.bz2"
        fi
    done
    
    available=$(${BEE_LIBEXECDIR}/bee-list --available "${search}")
    subsearch=( $(beecut -d '-' "${search}") )
    nss=${#subsearch[*]}
    
    install_candidate=
    
    for i in $(seq $(($nss-1)) -1 1) ; do
        debug_msg "checking subsearch $i: ${subsearch[$i]}"
        
        sss=${subsearch[$i]}
        
        found=
        for a in ${available} ; do
            p=$(beeversion --pkgfullname "${a}")
            debug_msg "  against $p ($a)"
            
            if [ "${p}" = "${sss}" ] ; then
                debug_msg "FOUND: pkgfullname(${p}): ${a}"
                if [ "${p}" = "${search}" ] ; then
                    debug_msg "  => INSTALL(max of ${a})"
                    install_candidate="${install_candidate} ${a}"
                elif begins_with "${a}" "${search}" ; then 
                    debug_msg "  => MAY INSTALL(max of ${a})"
                    install_candidate="${install_candidate} ${a}"
                fi
            elif begins_with "${p}" "${sss}" ; then
                debug_msg "FOUND: submatch(${p}): ${a}"
                install_match="${install_match} ${a}"
            fi
        done
        
        if [ -n "${install_candidate}" ] ; then
            debug_msg "install candidates: ${install_candidate}"
            to_bee_installed=$(beeversion -max ${install_candidate})
            debug_msg "   => and the winner is:  ${to_bee_installed}"
            pkg_install ${to_bee_installed}
            break
        fi
        
        if [ -n "${install_match}" ] ; then
            debug_msg "matches: ${install_match}"
            print_pkg_list $(${BEE_LIBEXECDIR}/bee-list -a "${search}")
            break
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
    
    local pname=$(beeversion --pkgfullname ${pkg})
    local list
    
    for i in $(${BEE_LIBEXECDIR}/bee-list -i "${pname}") ; do
        local installed=$(beeversion --pkgfullname ${i})
        if [ "${installed}" = "${pname}" ] ; then
            list="${list:+${list} }${i}"
        fi
    done
        
    echo "${list}"
}

###############################################################################
### check_installed ###
##
## IN: full_packagename
##
## OUT: n/a - only returns if package is not installed in any version
##
## DESCRIPTION:
##     - print installed versions of "full_packagename"
##     - exit(2) if installed version of "full_packagename" exists
##
check_installed() {
    local pkgname=${1}
    
    local installed=$(get_installed_versions ${pkgname})
    
    if [ "${installed}" != "" ] ; then
        for i in ${installed} ; do
            if [ ${i} = ${pkgname} ] ; then
                echo "[  already installed  ] ${i}"
            else
                echo "[alternative installed] ${i}"
            fi
        done
        exit 2;
    fi
}

###############################################################################
### do_install ###
##
## IN: filename
##
## OUT: n/a
##
## DESCRIPTION:
##     - calls check_installed which exits if package is already installed
##       (skip test if -f option was given on commandline)
##     - installs package
##
do_install() {
    local file=${1}
    local pkg=$(echo ${file} | 
                  sed -e 's,\.tar,,g;s,\.bz2,,g;s,\.gz,,g' \
                      -e 's,\.[bi]ee,,g' \
                      -e 's,.*/,,')
    debug_msg "do_install file=${file}"
    debug_msg "do_install pkg=${pkg}"
    
    if [ "${OPT_F}" = "0" ] ; then
        check_installed ${pkg}
    fi
    
    # create bee-filename
    BEE="$(beeversion ${pkg} --format="%F").bee"
    
    taraction="-x"
    
    [ "${NOOP}" = "yes" ] && taraction="-t"
    
    echo "installing ${file} .."
    
    start_cmd tar ${taraction} -vvPf ${file} \
        --transform="s,^FILES$,${BEE_METADIR}/${pkg}/FILES," \
        --transform="s,^BUILD$,${BEE_METADIR}/${pkg}/${BEE}," \
        --transform="s,^META$,${BEE_METADIR}/${pkg}/META," \
        --transform="s,^PATCHES,${BEE_METADIR}/${pkg}/PATCHES," \
        --show-transformed-names
    
    exit $?
}

###############################################################################
### print_pkg_list ###
##
## args: $LIST
##
##
##
print_pkg_list() {
    local -i f=0
    
    if [ ! "${1}" ] ; then
        echo "no matching packages found in repository.."
        exit 4
    fi
    
    for p in ${@} ; do
        local pkgname=$(basename $(basename $(basename ${p} .tar.bz2) .iee) .bee)
        local installed=$(get_installed_versions ${pkgname})
        
        local status
        for i in ${installed} ; do
            if [ ${i} = ${pkgname} ] ; then
                status="*"
            else
                status=${status:--}
            fi
        done
        status=${status:- }
        if [ "${status}" = " " -o ${OPT_A} -gt 0 ] ; then
            echo " [${status}] ${pkgname}"
            f=$f+1
        fi
        unset status
    done
    
    echo "${f} of ${#} packages displayed matching search criteria."
    
    if [ ${f} -eq 0 ] ; then
        echo "no new packages found in repository. try -v to list already installed packages."
        exit 5
    fi
}

###############################################################################
###############################################################################
###############################################################################


options=$(getopt -n bee_install \
                 -o iavfuh \
                 --long install,upgrade,verbose,all,force,help \
                 --long debug,noop \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

declare -i OPT_A=0
declare -i OPT_F=0

DEBUG=":"

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
    -u|--upgrade)
      shift
      pkg_upgrade $@
      exit 0
      ;;    
    -h|--help)
      usage
      exit 0
    ;;
    -i|--install)
      shift
      ;;
    --debug)
        shift
        DEBUG="yes"
        ;;
    --noop)
        shift
        NOOP="yes"
        ;;
    *)
      shift
      if [ -z ${1} ] ; then
           usage
           exit 1
      fi
      pkg_install_all ${@}
      exit 0;
      ;;
  esac
done
