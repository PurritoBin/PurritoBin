#!/bin/sh

# This file contains two functions which allow uploading a standard message
# and one which encrypts your message before sending it
# Requirements for encrypted message sending:
# - LibreSSL / OpenSSL / GnuTLS


# POSIX shell client to upload standard message
purr() {
	url="$(curl --silent --data-binary "@${1:-/dev/stdin}" bsd.ac:42069)"
	echo "${url}"
}


# POSIX shell client to upload encrypted message
meow() {
	# we need to generate a 256 byte random key
	# for using the aes-256-cbc cipher
	key="$(openssl rand -hex 32)"
	# calculate its encryption and upload it
	# iv doesn't matter as this is a one time use key
	# and we dont worry about salting and stuff
	url="$(openssl enc -aes-256-cbc -K ${key} -iv 00000000000000000000000000000000 -e -base64 -A < ${1:-/dev/stdin} | purr)"
	echo "${url%\/*}/paste.html#${url##*\/}_${key}"
}
