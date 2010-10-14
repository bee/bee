#!/bin/bash

usage() {
    echo "usage: $0 <option> <arguments>"
    echo "possible options are:"
    echo "  init"
    echo "  install"
    echo "  remove"
    echo "  check"
    echo "  version"
}


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
    *)
        if [ -n "${@}" ] ; then
            echo "$1 is not a known option.."
        fi
        usage
        ;;
esac
