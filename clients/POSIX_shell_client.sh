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
	iv="$(openssl rand -hex 12)"
	# calculate its encryption and upload it
	url="$(openssl enc -aes-256-cbc -K ${key} -iv ${iv} -e -base64 -A < ${1:-/dev/stdin} | purr)"
	printf %s\\n "${url%\/*}/paste.html#${url##*\/}_${key}_${iv}"
}


# POSIX shell client to decrypt the message
meowd() {
	url="$1"
	baseurl="${url%\/*}"
	vals="${url##*\#}"
	IFS="_" set -- $vals
	encrypteddata="$(curl --silent ${baseurl}/$1)"
	printf %s\\n $encrypteddata | openssl enc -aes-256-cbc -base64 -d -K $2 -iv $3
}
