#!/bin/bash

case "$1" in
    init)
        shift
        beeinit $@
        ;;
    install)
        shift
        beeinstall $@
        ;;
    remove)
        shift
        beeremove $@
        ;;
    check)
        shift
        beecheck $@
        ;;
    version)
        shift
        beeversion $@
        ;;
    *)
        echo "$1 is not a known option.."
        echo "possible options are:"
        echo " * init"
        echo " * install"
        echo " * remove"
        echo " * check"
        echo " * version"
        ;;
esac
