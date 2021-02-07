#!/bin/sh

. ./common.sh

set -e

P_TMPDIR=$(mktemp -d -t)
P_PORT="$(shuf -i 1500-65536 -n 1)"

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -m 11 &
P_ID=$!
P_RACING=

# should be enough
sleep 2

test_single_paste() {
    P_DATA="SOME_RANDOM_TEST_DATA"
    P_PASTE=$(printf %s\\n "${P_DATA}" | purr)
    if [ -z "${P_PASTE}" || ! -f "${P_PASTE}" ]; then
        return 1
    fi
    printf %s\\n "${P_DATA}" | diff -u "${P_PASTE}" -
}

P_CONCUR=${P_CONCUR:-100}

P_LOOP=1
while [ ${P_LOOP} -le 100 ]; do
    test_single_paste >/dev/null &
    P_LOOP=$((P_LOOP+=1))
done

# should be enough
sleep 2

set +e
pinfo "${0}: success"
