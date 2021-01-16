#!/bin/sh

# This file contains two functions which allow uploading a standard message
# and one which encrypts your message before sending it
# Requirements for encrypted message sending:
# - LibreSSL / OpenSSL / GnuTLS


# POSIX shell client to upload standard message
purr() {
        curl --silent --data-binary "@${1:-/dev/stdin}" bsd.ac:42069
}

# POSIX shell client to upload encrypted message
meow() {
        # we need to generate a 256 byte random key
        # for using the aes-256-cbc cipher
        key="$(openssl rand -hex 32)"
        iv="$(openssl rand -hex 16)"
        # calculate its encryption and upload it
        url="$(openssl enc -aes-256-cbc -K ${key} -iv ${iv} -e -base64 -A < ${1:-/dev/stdin} | purr)"
        printf %s\\n "${url%\/*}/paste.html#${url##*\/}_${key}_${iv}"
        unset key iv url
}


# POSIX shell client to decrypt the message
meowd() {
        url="$1"
        baseurl="${url%\/*}"
        vals="${url##*\#}"
        paste=$(printf %s\\n "${vals}" | cut -d'_' -f1)
        key=$(printf %s\\n "${vals}" | cut -d'_' -f2)
        iv=$(printf %s\\n "${vals}" | cut -d'_' -f3)
        encrypteddata="$(curl --silent ${baseurl}/$paste)"
        printf %s\\n $encrypteddata | openssl enc -aes-256-cbc -base64 -d -K $key -iv $iv
        unset url baseurl vals paste key iv
}
