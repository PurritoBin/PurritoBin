#!/bin/sh

purr() {
    # get the data, either from file or from stdin
    data=$(cat ${1:-/dev/stdin})
    # generate a random 256 bit key for AES
    key=$(openssl rand -base64 32)
    # calculate its encryption and store it
    url=$(printf "%s" "$data" | openssl enc -aes-256-cbc -pbkdf2 -salt -pass pass:"${key}" | nc bsd.ac 42069)
    echo "${url}#${key}"
}

purr
