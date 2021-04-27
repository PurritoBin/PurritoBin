#!/bin/sh

. ./common.sh

set -e

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -z "${P_TMPDBDIR}" -i 127.0.0.1 -p "${P_PORT}" -n localhost -k PB.key -c PB.crt -l &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n `${SEQ} 1 "${P_CONCUR}"` | xargs -n 1 -P "${P_CONCUR}" sh "HELPER_${0}" "${P_PORT}"

# should be enough
sleep 2

set +e
pinfo "${0}: success"
