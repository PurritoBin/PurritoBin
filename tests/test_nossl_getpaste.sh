#!/bin/sh

set -e

. ./common.sh

P_RACING=1
${PURRITO} -d "${P_TMPDIR}/" -s "${P_TMPDIR}" -i 127.0.0.1 -p "${P_PORT}" -t &
P_ID=$!
P_RACING=

# should be enough
sleep 2

printf %s\\n "THISISINDEX" > "${P_TMPDIR}/index.html"

curl --silent --fail "localhost:${P_PORT}/" | diff "${P_TMPDIR}/index.html" -
curl --silent --fail "localhost:${P_PORT}"  | diff "${P_TMPDIR}/index.html" -

printf %s\\n "THISISRANDOMPASTE" > "${P_TMPDIR}/somethingrandom"
curl --silent --fail "localhost:${P_PORT}/somethingrandom" | diff "${P_TMPDIR}/somethingrandom" -

# should be enough
sleep 2

set +e
pinfo "${0}: success"
