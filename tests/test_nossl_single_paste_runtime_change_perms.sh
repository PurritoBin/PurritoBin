#!/bin/sh

. ./common.sh
. ./common_functions.sh

set -e

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -z "${P_TMPDBDIR}" -i 127.0.0.1 -p "${P_PORT}" &
P_ID=$!
P_RACING=

# should be enough
sleep 2

chmod -w "${P_TMPDIR}"

P_DATA="SOME_RANDOM_TEST_DATA"

set +e
P_PASTE=$(printf %s\\n "${P_DATA}" | purr)
set -e

if [ ! -z "${P_PASTE}" ]; then
    exit 1
fi

chmod +w "${P_TMPDIR}"

set +e
pinfo "${0}: success"
