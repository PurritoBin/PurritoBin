if [ -n "${TERM}" ] && [ "${TERM}" != "dumb" ]; then
    export __bold="$(tput bold)" \
    __green="$(tput setaf 2)" \
    __reset="$(tput sgr0)"
fi

pinfo()  { printf %s\\n "${__green}${__bold}LOG${__reset}: $*"; }

purr() {
        curl --silent --data-binary "@${1:-/dev/stdin}" "127.0.0.1:${P_PORT}"
}

trap_exit() {
    [ "${P_RACING}" ] && P_ID=$!
    [ -e "${P_TMPDIR}" ] && rm -rf "${P_TMPDIR}"
    [ "${P_ID}" ] && kill "${P_ID}"
}

trap trap_exit EXIT INT TERM
