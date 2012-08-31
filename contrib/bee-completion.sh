#!/bin/bash

_bee_completion()
{
    COMPREPLY=()

    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    bee_tools="cache check download init install list query remove update"

    if [ "${prev}" = "bee" ]; then
        COMPREPLY=( $(compgen -W "${bee_tools}" -- ${cur} ) )
        return 0
    fi

    packages="$(bee list -a)"
    COMPREPLY=( $(compgen -W "${packages}" -- ${cur}) )

    return 0
}

complete -f -F _bee_completion bee
