#!/bin/sh

. ./common.sh

set -e

P_TMPDIR=$(mktemp -d -t)
P_PORT="$(${SHUF} -i 1500-65536 -n 1)"
: ${P_CONCUR=100}

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n `${SEQ} 1 "${P_CONCUR}"` | xargs -n 1 -P "${P_CONCUR}" sh test_concurrent_pastes_helper.sh "${P_PORT}"

# should be enough
sleep 2

set +e
pinfo "${0}: success"
