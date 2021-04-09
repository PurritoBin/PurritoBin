#!/bin/sh

. ./common.sh
. ./common_functions.sh

set -e

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -m 10 &
P_ID=$!
P_RACING=

# should be enough
sleep 2

P_DATA="SOME_RANDOM_TEST_DATA"

set +e
P_PASTE=$(printf %s\\n "${P_DATA}" | purr)
set -e

# P_PASTE should be blank
if [ ! -z "${P_PASTE}" ]; then
    exit 1
fi

set +e
pinfo "${0}: success"
