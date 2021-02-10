# test controllables

: ${SHUF=shuf}
: ${SEQ=seq}
: ${P_PORT=$(${SHUF} -i 1500-65536 -n 1)}
: ${P_TMPDIR=$(mktemp -d -t)}
: ${P_DATA=$(mktemp -p ${P_TMPDIR})}
: ${P_CONCUR=100}
: ${P_MAXSIZE=100}

### needed for github actions
if [ ${GH_ACTIONS} = "YES" ]; then
    P_DD_FLAGS="iflag=fullblock"
fi

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
    [ "${P_ID}" ] && kill "${P_ID}"
}

trap trap_exit EXIT INT TERM
