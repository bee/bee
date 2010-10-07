#!/bin/bash
#
# bee-list - list pkgs
# Copyright (C) 2009-2010 
#       Marius Tolzmann <tolzmann@molgen.mpg.de>
#       Tobias Dreyer <dreyer@molgen.mpg.de>
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

VERSION=0.1

: ${BEEFAULTS=/etc/bee.defaults}

if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

if [ -z "${BEEPKGSTORE}" -a ! -z "${PKGDIR}" ] ; then
    echo >&2 "WARNING: PKGDIR is obsolete; please use BEEPKGSTORE instead."
    BEEPKGSTORE=${PKGDIR}
fi

: ${BEEMETADIR:=/usr/share/bee}
: ${BEEPKGSTORE:=/usr/src/bee/pkgs}

##### usage ###############################################################
usage() {
    echo "bee-list v${VERSION} 2009-2010"
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
get_name_from_pkg() {
    beeversion $1 --pkgfullname
}

###############################################################################
### get_pkg_list ###
##
## IN: 
##
##
##
get_pkg_list_repository() {
    local search=${1}
    
    local hits arch
    
    if [ ${OPT_A} -le '1' ] ; then
        arch="\.(any|noarch|$(arch))"
    fi
    
    hits=$(find ${BEEPKGSTORE} -mindepth 1 \
             | egrep "${arch}\.iee" \
	     | egrep "${search}" \
	     | sort)

    echo ${hits}
}

list_all_available_pkgs() {
    local arch=""
    
    if [ ${OPT_A} -lt '1' ] ; then
        arch="\.(any|noarch|$(arch))"
    fi
    
    find ${BEEPKGSTORE} -mindepth 1 -type f -printf "%f\n" \
        | egrep "${arch}\.iee" \
        | sort \
        | sed -e 's#\.gz$##' \
              -e 's#\.bz2##' \
              -e 's#\.iee\.tar$##'
}

get_installed_versions() {
    search=${1}
    
    for pkg in $(list_all_installed_pkgs | egrep "^${search}-") ; do
        pname=$(beeversion --pkgfullname "${pkg}")
        
        if [ "${pname}" == "${search}" ] ; then
            echo "${pkg}"
        fi
        
    done
}

get_install_status() {
    pkg=${1}
    
    pkgfullname=$(beeversion --pkgfullname ${pkg})
    
    p=""
    
    for p in $(get_installed_versions "${pkgfullname}") ; do
        echo "installed '${p}' version of '${pkg}' .."
        if beeversion "${p}" --gt "${pkg}" ; then
            echo "  - is newer"
        elif beeversion "${p}" --lt "${pkg}" ; then
            echo "  - is older"
        elif beeversion "${p}" --eq "${pkg}" ; then
            echo "  - is equal"
        else 
            echo "ERROR"
        fi
    done
    
    if [ -z "${p}" ] ; then
        echo "${pkgfullname} not installed"
    else
        echo "ERROR '${p}'"
    fi
    
}

list_available_pkgs() {
    for pkg in $(list_all_available_pkgs) ; do
        
        get_install_status "${pkg}"
        
        status=$(get_install_status "${pkg}")
        
        case "${status}" in
            update)
                echo "U ${pkg}"
                ;;
            installed)
                echo "I ${pkg}"
                ;;
            notinstalled)
                echo "N ${pkg}"
                ;;
            *)
                echo >&2 "unknown status(${pkg}) = '${status}'"
                ;;
        esac
        
        
    done
    
}


list_all_installed_pkgs() {
    find ${BEEMETADIR} -maxdepth 1 -mindepth 1 -type d -printf "%f\n" \
        | sort
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
	local pkgname=$(basename $(basename ${p} .tar.bz2) .iee)
	
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
                 -o iah \
                 --long installed,available,all,help \
                 -- "$@")
if [ $? != 0 ] ; then
  iee_usage
  exit 1
fi
eval set -- "${options}"

declare -i OPT_A=0
declare -i OPT_F=0

mode=

while true ; do
    opt=${1}
    shift
    
    case "${opt}" in
        --all)
            OPT_A=${OPT_A}+1
            ;;
        -i|--installed)
            mode="installed"
            ;;
        -a|--available)
            mode="available"
            ;;
        --)
            break
            ;;
        *)
            echo "Internal error!"
            exit 1
            ;;
    esac
done

case "${mode}" in
    available)
        list_available_pkgs
        ;;
    *)
        list_all_installed_pkgs
        ;;
esac



# list available
#   arch
#   only list latest version of a pkg for each arch
#   skip installed...

