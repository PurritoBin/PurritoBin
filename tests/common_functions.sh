purr() {
    curl --max-time "${P_MAXTIME:-30}" --silent --data-binary "@${1:-/dev/stdin}" "localhost:${P_PORT}/day"
}

spurr() {
    curl --max-time "${P_MAXTIME:-30}" --silent --cacert "${P_CRT}" --data-binary "@${1:-/dev/stdin}" "https://localhost:${P_PORT}"
}
