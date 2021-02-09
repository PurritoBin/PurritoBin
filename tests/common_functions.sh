purr() {
    curl --max-time "${P_MAXTIME:-30}" --silent --data-binary "@${1:-/dev/stdin}" "localhost:${P_PORT}"
}

spurr() {
    curl --max-time "${P_MAXTIME:-30}" --silent --cacert PB.crt --capath "$(pwd)" --data-binary "@${1:-/dev/stdin}" "https://localhost:${P_PORT}"
}
