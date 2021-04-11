#!/bin/sh

. ./common.sh
. ./common_functions.sh

set -e

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -z "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -m $((${P_MAXSIZE} * 1024 * 1024))  &
P_ID=$!
P_RACING=

# should be enough
sleep 2

dd if=/dev/random of="${P_DATA}" bs=1M count=$((${P_MAXSIZE} - 1)) ${P_DD_FLAGS}

P_PASTE=$(purr "${P_DATA}")

# P_PASTE is not set or empty
# OR
# P_PASTE is not a file
if [ -z "${P_PASTE}" ] || [ ! -f "${P_PASTE}" ]; then
    exit 1
fi

diff -u "${P_PASTE}" "${P_DATA}"

set +e
pinfo "${0}: success"
