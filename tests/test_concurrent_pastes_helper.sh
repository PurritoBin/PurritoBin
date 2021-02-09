. ./common.sh

P_PORT=$1
P_PURR=${2:-purr}

P_DATA="SOME_RANDOM_TEST_DATA"
P_PASTE=$(printf %s\\n "${P_DATA}" | ${P_PURR})
if [ -z "${P_PASTE}" ] || [ ! -f "${P_PASTE}" ]; then
    exit 1
fi
printf %s\\n "${P_DATA}" | diff -u "${P_PASTE}" -
