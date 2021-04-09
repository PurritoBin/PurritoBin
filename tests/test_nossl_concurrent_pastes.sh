#!/bin/sh

set -e

. ./common.sh

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n `${SEQ} 1 "${P_CONCUR}"` | xargs -n 1 -P "${P_CONCUR}" sh "HELPER_${0}" "${P_PORT}"

# should be enough
sleep 2

set +e
pinfo "${0}: success"
