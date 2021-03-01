#!/bin/sh

set -e

. ./common.sh

P_RACING=1
../purrito -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -t -n localhost -k "PB.key" -c "PB.crt" -l &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n "THISISINDEX" > "${P_TMPDIR}/index.html"

curl --silent --cacert PB.crt --capath "$(pwd)" --fail "https://localhost:${P_PORT}/" | diff "${P_TMPDIR}/index.html" -
curl --silent --cacert PB.crt --capath "$(pwd)" --fail "https://localhost:${P_PORT}"  | diff "${P_TMPDIR}/index.html" -

printf %s\\n "THISISRANDOMPASTE" > "${P_TMPDIR}/somethingrandom"
curl --silent --cacert PB.crt --capath "$(pwd)" --fail "https://localhost:${P_PORT}/somethingrandom" | diff "${P_TMPDIR}/somethingrandom" -

# should be enough
sleep 2

set +e
pinfo "${0}: success"
