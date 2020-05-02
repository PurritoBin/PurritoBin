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
	# change this number to a larger one if you want to
	key="$(openssl rand -hex 8)"
	# calculate its encryption and upload it
	# pbkdf2 is considered a lot more secure
	url="$(openssl enc -aes-256-cbc -pass pass:"${key}" -e -base64 < ${1:-/dev/stdin} | purr)"
	echo "${url%\/*}/paste.html#${url##*\/}_${key}"
}
