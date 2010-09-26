#!/bin/bash


BEESEP=beesep


###############################################################################
###############################################################################
###############################################################################

VERSION=0.1

: ${BEEFAULTS:=/etc/bee.defaults}

if [ -e ${BEEFAULTS} ] ; then
    . ${BEEFAULTS}
fi

: ${METADIR:=/usr/share/bee}

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
    installed=$(get_installed_versions ${1})
    
    if [ ! "${installed}" -a $OPT_F -gt 0 ] ; then
        installed=$(get_pkg_list_installed ${1})
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
    installed=$(get_installed_versions ${1})
    
    if [ ! "${installed}" -a $OPT_F -gt 0 ] ; then
        installed=$(get_pkg_list_installed ${1})
    fi
    
    if [ "${installed}" ] ; then
        for i in ${installed} ; do
	    do_check "${i}"
	done
	return 0
    fi
    
    installed=$(get_pkg_list_installed ${1})
    
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
    local filesfile=${METADIR}/${pkg}/FILES
    
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
	
#	echo ${file}
	
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
    
    local filesfile=${METADIR}/${pkg}/FILES
    
    echo "checking ${pkg} .."
    
    for line in $(cat ${filesfile}) ; do 
	eval $(${BEESEP} ${line})

        # save and strip possible symbolic link destination..
	symlink=${file#*//}
        file=${file%%//*}

	if [ ! -e "${file}" ] ; then
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
    
    local pname=$(get_name_from_pkg ${pkg})
    local list
    
    for i in $(get_pkg_list_installed "${pname}") ; do
        local installed=$(get_name_from_pkg ${i})
	if [ "${installed}" = "${pname}" ] ; then
	    list="${list:+${list} }${i}"
	fi
    done
        
    echo "${list}"
}

###############################################################################
### get_pkg_list_installed ###
##
## IN: 
##
##
##
get_pkg_list_installed() {
    local search=${1}
    
    local hits arch
    
    hits=$(find ${METADIR} -maxdepth 1 -mindepth 1 -type d -printf "%f\n" \
             | egrep "${search}" | sort)
    
    echo ${hits}
}

###############################################################################
##
##

#sub1-sub2-subn-name-V.V.V.V-R.A.iee.tar.bz2

get_fullversion_from_pkg() {
    echo $(echo $1 | sed -e 's,^\(.*\)-\(.*\)-\(.*\)\.\(.*\)$,\2-\3,' - )
}

###############################################################################
##
##
get_name_from_pkg() {
    echo $(echo $1 | sed -e 's,^\(.*\)-\(.*\)-\(.*\)$,\1,' - )
}



###############################################################################
##
##
options=$(getopt -n bee_check \
                 -o iadvfuh \
                 --long install,upgrade,verbose,all,force,help \
                 -- "$@")
if [ $? != 0 ] ; then
  iee_usage
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
    -u|--upgrade)
      shift
      pkg_upgrade $@
      exit 0
      ;;    
    -h|--help)
      iee_usage
      exit 0
    ;;
    -i|--install)
      shift
      ;;
    *)
      shift
      pkg_check_all ${@}
      
      exit 0;
      ;;
  esac
done
