# test controllables

: ${SHUF=shuf}
: ${SEQ=seq}
: ${P_PORT=$(${SHUF} -i 1500-65536 -n 1)}
: ${P_TMPDIR=$(mktemp -d --tmpdir)}
: ${P_TMPDBDIR=$(mktemp -d --tmpdir)}
: ${P_DATA=$(mktemp -p ${P_TMPDIR})}
: ${P_CONCUR=10}
: ${P_MAXSIZE=5}
: ${PURRITO=../purrito}

##########################

if [ -n "${TERM}" ] && [ "${TERM}" != "dumb" ]; then
    export __bold="$(tput bold)" \
    __green="$(tput setaf 2 0 0)" \
    __reset="$(tput sgr0)"
fi

pinfo()  { printf %s\\n "${__green}${__bold}LOG${__reset}: $*"; }

trap_exit() {
    [ "${P_RACING}" ] && P_ID=$!
    [ -e "${P_TMPDIR}" ] && rm -rf "${P_TMPDIR}"
    [ -e "${P_TMPDBDIR}" ] && rm -rf "${P_TMPDBDIR}"
    [ "${P_ID}" ] && kill "${P_ID}"
}

trap trap_exit EXIT INT TERM
