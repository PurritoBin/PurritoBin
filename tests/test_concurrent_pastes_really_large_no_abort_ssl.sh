#!/bin/sh

. ./common.sh

set -e

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -m $((${P_MAXSIZE} * 1024 * 1024)) -n localhost -k PB.key -c PB.crt -l &
P_ID=$!
P_RACING=

# should be enough
sleep 2

dd if=/dev/random of="${P_DATA}" bs=1M count=$((${P_MAXSIZE} - 1)) iflag=fullblock

printf %s\\n `${SEQ} 1 "${P_CONCUR}"` | xargs -n 1 -P "${P_CONCUR}" sh test_concurrent_pastes_really_large_no_abort_ssl_helper.sh "${P_PORT}" "${P_DATA}"

# should be enough
sleep 2

set +e
pinfo "${0}: success"
