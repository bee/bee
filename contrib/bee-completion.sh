#!/bin/bash

_bee_completion()
{
    COMPREPLY=()

    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    options="init install remove check query list"
    ls=$(ls $pwd)

    if [ "${prev}" = "bee" ]; then
        COMPREPLY=($(compgen -W "${options} ${ls}" -- ${cur}) )
        return 0
    fi

    packages=$(bee list -a)
    COMPREPLY=($(compgen -W "${packages} ${ls}" -- ${cur}) )

    return 0
}

complete -F _bee_completion bee
