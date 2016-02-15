_cape_completion()
{
    local cur=${COMP_WORDS[COMP_CWORD]}
    COMPREPLY=( $(compgen -W "compile edit ls rm" -- $cur) )
}
complete -F _cape_completion cape
