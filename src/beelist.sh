#!/bin/bash
set -e

VERSION=0.1

: ${PKGSTORE:=/usr/src/bee/pkgs/}
: ${BEEMETADIR:=/usr/share/bee/}
: ${BEESTORE:=/usr/src/bee/bees/}

ARCH=$(arch)

usage() {
    echo "bee-list v${VERSION} 2009-2010"
    echo
    echo "Usage: $0 [action] [options] <pattern>"
    echo
    echo "action .."
    echo "    -a | --available          list available pkgs matching pattern"
    echo "    -i | --installed          list installed pkgs matching pattern"
    echo "    -b | --bees               list beefiles matching pattern"
    echo
    echo "options .."
    echo "    -l [timestamp|version]    filter list for latest pkg"
}

intersection() {
    a=(${1})
    b=(${2})
    
    for i in "${a[@]}" "${b[@]}" ; do
        echo $i
    done | sort | uniq -d
}

unionset() {
    a=(${1})
    b=(${2})
    
    for i in "${a[@]}" "${b[@]}" ; do
        echo $i
    done | sort | uniq
}

without() {
    a=(${1})
    b=(${2})
    ${DEBUG} "${a[@]}" >&2
    for i in "${b[@]}" ; do
        ${DEBUG} "i $i" >&2
        pn=$(beeversion --pkgname ${i})
        a=($(echo "${a[*]}" | sed -e  "s@${pn}[^[:blank:]]*@@g"))
        ${DEBUG} "${a[@]}" >&2
    done
    
    echo "${a[@]}"
}

upgrades() {
    a=(${1})
    b=(${2})
    
    for i in "${a[@]}" ; do
        pn=$(beeversion --pkgname ${i})
        ${DEBUG} "${pn}" >&2
        l=$(basename $(beeversion -max ${i} $(echo ${PKGSTORE}${pn}*)) | sed -e "s,\.bz2$,," -e "s,\.gz$,," -e "s,\.bee.tar$,," -e "s,\.iee.tar$,,")
        ${DEBUG} "${l}" >&2
        if [ "${i}" != "${l}" ] ; then
            echo "${l}"
        fi
    done
}

get_view() {
    filter=${1}
    shift
    list=(${@})
    
    if [ -z "${list[*]}" ] ; then
        list=""
    fi
    
    for p in "${list[@]}" ; do
        matches=($(get_matches ${filter} ${p}))
        for m in "${matches[@]}" ; do
            if [ $(echo ${view} | egrep -c ${m}) -eq 0 ] ; then
                view="${view:+${view} }${m}"
            fi
        done
    done
    
    for v in ${view} ; do
        echo "${v}"
    done
}

get_matches() {
    filter=${1}
    pattern=${2}
    
    cnt=0
    inst=($(pattern2list ${BEEMETADIR} ${pattern}))
    ${DEBUG} "inst .. ${inst[@]}" >&2
    avail=($(pattern2list ${PKGSTORE} ${pattern}))
    ${DEBUG} "avail .. ${avail[@]}" >&2
    while [ ${#filter} -gt ${cnt} ] ; do
        case "${filter:${cnt}:1}" in
            a)
                list=(${avail[@]})
                ;;
            i)
                list=(${inst[@]})
                ;;
            u)
                list=($(upgrades "${inst[*]}" "${avail[*]}"))
                ;;
            n)
                list=($(without "${avail[*]}" "${inst[*]}"))
                ;;
        esac
        cnt=$((${cnt}+1))
    done
    
    #debug output
    for m in "${list[@]}" ; do
        ${DEBUG} "list .. $m" >&2
    done
    
    echo "${list[@]}"
}

pattern2list() {
    path=${1}
    pattern=${2}
    
    echo $(find ${path} -mindepth 1 -maxdepth 1 -printf "%f\n" | sed -e "s@\.iee\.tar\..*\$@@" -e "s@\.bee\.tar\..*\$@@" | egrep "${pattern}" | sort )
}

options=$(getopt -n beelist \
                 -o aiunh \
                 --long available,installed,upgrades,not-installed,help,debug \
                 -- "$@")
if [ $? != 0 ] ; then
  usage
  exit 1
fi
eval set -- "${options}"

DEBUG=":"
while true ; do
    case "$1" in
        -h|--help)
            usage
            exit 0
            ;;
        -a|--available)
            shift
            filter="a"
            ;;
        -i|--installed)
            shift
            filter="i"
            ;;
        -u|--upgrades)
            shift
            filter="u"
            ;;
        -n|--not-installed)
            shift
            filter="n"
            ;;
        --debug)
            shift
            DEBUG="echo"
            ;;
        --)
            shift
            if [ ! ${filter} ] ; then
                usage
                exit 1
            fi
            get_view ${filter} ${@}
            exit 0;
            ;;
    esac
done
