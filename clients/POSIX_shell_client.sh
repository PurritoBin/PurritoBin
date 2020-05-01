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
	# generate a random 256 bit key for AES
	key="$(openssl rand -base64 32)"
	# calculate its encryption and upload it
	# pbkdf2 is considered a lot more secure
	url="$(openssl enc -aes-256-cbc -pbkdf2 -salt -pass pass:"${key}" < ${1:-/dev/stdin} | purr)"
	echo "${url%\/*}/paste.html\#${url##*\/}_${key}"
}
