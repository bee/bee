#!/bin/bash

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
        echo "$1 is not a known option.."
        echo "possible options are:"
        echo " * init"
        echo " * install"
        echo " * remove"
        echo " * check"
        echo " * version"
        ;;
esac
