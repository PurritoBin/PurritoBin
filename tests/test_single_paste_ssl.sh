#!/bin/sh

set -e

. ./common.sh

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -n localhost -k "PB.key" -c "PB.crt" -l &
P_ID=$!
P_RACING=

# should be enough
sleep 2

P_DATA="SOME_RANDOM_TEST_DATA"

P_PASTE=$(printf %s\\n "${P_DATA}" | spurr)

# P_PASTE is not set or empty
# OR
# P_PASTE is not a file
if [ -z "${P_PASTE}" ] || [ ! -f "${P_PASTE}" ]; then
    exit 1
fi

printf %s\\n "${P_DATA}" | diff "${P_PASTE}" -

set +e
pinfo "${0}: success"
