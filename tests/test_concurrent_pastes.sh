#!/bin/sh

. ./common.sh

set -e

P_TMPDIR=$(mktemp -d -t)
P_PORT="$(shuf -i 1500-65536 -n 1)"

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n `seq 1 100` | xargs -n 1 -P 100 sh test_concurrent_pastes_helper.sh "${P_PORT}"

# should be enough
sleep 2

set +e
pinfo "${0}: success"
