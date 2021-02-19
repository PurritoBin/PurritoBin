. ./common_functions.sh

P_PORT=$1
P_DATA=$2

P_PASTE=$(cat "${P_DATA}" | spurr)
if [ -z "${P_PASTE}" ] || [ ! -f "${P_PASTE}" ]; then
    exit 1
fi

diff -u "${P_PASTE}" "${P_DATA}"
