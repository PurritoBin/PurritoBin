#!/bin/sh

. ./common.sh

set -e

P_TMPDIR="$(mktemp -d -t)"
P_PORT="$(${SHUF} -i 1500-65536 -n 1)"

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -m 300000000 &
P_ID=$!
P_RACING=

# should be enough
sleep 2

P_DATA_FILE="$(mktemp -p ${P_TMPDIR} )"
dd if=/dev/random of="${P_DATA_FILE}" bs=4M count=80

set +e
P_PASTE=$(purr "${P_DATA_FILE}")
set -e

if [ ! -z "${P_PASTE}" ]; then
    exit 1
fi

set +e
pinfo "${0}: success"
