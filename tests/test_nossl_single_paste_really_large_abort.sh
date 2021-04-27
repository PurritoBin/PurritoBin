#!/bin/sh

. ./common.sh
. ./common_functions.sh

set -e

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -z "${P_TMPDBDIR}" -i 127.0.0.1 -p "${P_PORT}" -m $((${P_MAXSIZE} * 1024 * 1024)) &
P_ID=$!
P_RACING=

# should be enough
sleep 2

dd if=/dev/urandom of="${P_DATA}" bs=1M count=$((${P_MAXSIZE} + 1)) ${P_DD_FLAGS}

set +e
P_PASTE=$(purr "${P_DATA}")
set -e

if [ ! -z "${P_PASTE}" ]; then
    exit 1
fi

set +e
pinfo "${0}: success"
