#!/bin/sh

. ./common.sh

set -e -x

P_TMPDIR=$(mktemp -d -t)
P_PORT="$(shuf -i 1500-65536 -n 1)"

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" &
P_ID=$!
P_RACING=

P_DATA="SOME_RANDOM_TEST_DATA"

P_PASTE=$(printf %s\\n "${P_DATA}" | purr)

printf %s\\n "${P_DATA}" | diff "${P_PASTE}" -

set +e +x
pinfo "${0}: succeeded test"
