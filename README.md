# Purrito Bin  (=♡ᆺ♡=)
[![pipeline](https://github.com/PurritoBin/PurritoBin/workflows/pipeline/badge.svg)](https://github.com/PurritoBin/PurritoBin/actions?query=workflow:pipeline)
[![GitHub release (latest SemVer including pre-releases)](https://img.shields.io/github/v/release/PurritoBin/PurritoBin?include_prereleases)](https://github.com/PurritoBin/PurritoBin/releases)
[![GitHub license](https://img.shields.io/github/license/PurritoBin/PurritoBin.svg)](https://github.com/PurritoBin/PurritoBin/blob/master/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues-raw/PurritoBin/PurritoBin)](https://github.com/PurritoBin/PurritoBin/issues)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/PurritoBin/PurritoBin/issues)

[![docker](https://github.com/PurritoBin/docker/actions/workflows/docker.yml/badge.svg)](https://hub.docker.com/r/purritobin/purritobin)
[![docker pulls](https://img.shields.io/docker/pulls/purritobin/purritobin)](https://hub.docker.com/r/purritobin/purritobin)
[![image size](https://img.shields.io/docker/image-size/purritobin/purritobin/latest)](https://hub.docker.com/r/purritobin/purritobin)


ultra fast, minimalistic, encrypted command line paste-bin

[![Packaging status](https://repology.org/badge/vertical-allrepos/purritobin.svg)](https://repology.org/project/purritobin/versions)

## Features and Highlights

- *Very* lightweight: 2-3 MB of RAM on average.
- Listen on multiple address/port combinations, both IPv4 and IPv6.
- Configurable paste size limit.
- Auto-cleaning of pastes, with configurable paste lifetime at submission time:
   - `domain.tld/{day,week,month}`
   - `domain.tld/<time-in-minutes>`
   - `domain.tld/0` for a paste with infinite life.
- Paste storage in plain text, easy to integrate with all web servers (Apache, Nginx, etc.).
- Encrypted pasting similar to [PrivateBin](https://github.com/PrivateBin/PrivateBin).
- Optional **`https`** support for secure communication.
- Tiny code base, less than 1000 lines of code, for very easy auditing.
- Well documented, `man purrito`.


## Manual setup （ฅ＾・ﻌ・＾）ฅ

### Requirements

- [uSockets](https://github.com/uNetworking/uSockets/)
- [uWebSockets](https://github.com/uNetworking/uWebSockets/)
- [lmdbxx](https://github.com/hoytech/lmdbxx)

If these are not available in an OS's repositories, they can be manually installed by following the steps in the [GitHub workflow](https://github.com/PurritoBin/PurritoBin/actions?query=workflow:pipeline)

### Compilation

```
$ meson --prefix "/usr" -C build
$ ninja -C build
$ sudo ninja -C build install
```

### Usage

The server is run using the command `purrito`. To quickly view the available options:

```
$ purrito -h
usage: purrito [-abcdefghijklmnpqrstvwxz] -d domain [-a slug_characters]
               [-b max_database_size] [-c public_cert_file] [-e dhparams_file]
               [-f index_file] [-g slug_size] [-h] [-i bind_ip]
               [-j autoclean_interval] [-k private_key_file] [-l]
               [-m max_paste_size] [-n server name] [-p bind_port]
               [-q default_time_limit] [-r max_retries] [-s storage_directory]
               [-t] [-v header_value] [-w passphrase] [-x header]
               [-z database_directory]
```

For an indepth explanation, there is a man page provided.

```
$ man purrito
```

### Running the encrypted PurritoBin

Make sure that whatever link is provided to the `-d ` domain option, is also able to serve the `paste.html` from this repository.

## Client  (=｀ﻌ´=)

### Shell client definitions
Define these functions somewhere in the dot files of the desired shell (they work on all POSIX compliant shells).

```
: ${P_SERVER=https://bsd.ac}
: ${P_PORT=42069}
: ${P_MAXTIME=30}

# POSIX shell client to upload standard message
purr() {
	curl --silent --max-time "${P_MAXTIME}" --data-binary "@${1:-/dev/stdin}" "${P_SERVER}:${P_PORT}"
}

# POSIX shell client to upload encrypted message
meow() {
	# generate a 256 byte random key
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
	curl --max-time "${P_MAXTIME}" --write-out "\n" --silent "${baseurl}/${paste}" | openssl enc -aes-256-cbc -base64 -d -K ${key} -iv ${iv}
	unset url baseurl vals paste key iv
}
```

#### Client usage
The `stdout` from a command can be pasted or from the `cat/head/tail` of a file or the input from a file or from the command line.
```
$ purr c00lfile.txt
https://bsd.ac/purrit0

$ purr < h0tfile.txt
https://bsd.ac/purri1o

$ echo Hello world. | purr
https://bsd.ac/d4nklit

$ purr
This is my input
it can span multiple lines
<Ctrl-d to exit>
https://bsd.ac/curlpr0

$ meow
really cool paste
<Ctrl-d to exit>
https://bsd.ac/paste.html#329r1ml_9f5d0f2928b33c2a6a0752811170735af16c8eecfa208e1bdb84d831427be82b_fd579e101a3d31d0362f0ec6473573ad
```

### Encrypted Storage Clients  (=｀ᆺ├┬┴┬┴

In a encrypted storage setting, the paste is encrypted before sending it to the server.
Now the server will only be used as a storage bin and even in case of a non-https connection,
it is guaranteed that no one else will be able to read the data that was sent.

#### How does it work?
 Steps automatically done by the provided clients, on the client side:
 - Randomly generate an ecryption **key** and initialization vector **iv**.
 - Encrypt the **data** using said **key** and **iv**, the encrypted data is called the **cipher**.
 - Send the **cipher** to the pastebin and get a standard paste url as above, which will be converted to the form
 ```
    https://bsd.ac/paste.html#pasteID_key_iv
 ```
The standard client is `meow`, a companion to the above `purr`, which is also POSIX compliant and should work in all shells. It has exactly the same usage as `purr`, so please look at those examples to see how to use it.
The only dependency on the client side is the presence of [LibreSSL](https://www.libressl.org/) or [OpenSSL](https://www.openssl.org/), to do the encryption.

There will be other clients in the [clients](clients/) folder, added on demand for other platforms, which will do all this automatically.
Pull requests for other clients are highly welcome.

#### Why is this secure?
- All the encryption is done on the client side.
- Only the **cipher** is sent to PurritoBin.
- The url only contains the encrypted **cipher** and has no knowledge of the actual data.
- When visiting the html webpage the **key** is in the **hash property** of the webpage, which is never sent to the server.
- All decryption is done inside the browser, using javascript, [Crypto JS](https://github.com/brix/crypto-js) on the client side.

NOTE: Anyone who has the full link is going to be able to read the decrypted text. Only send the full hash included url to trusted parties, through a secure communication channel.


## Design principles
The aim is to follow the [KISS](https://en.wikipedia.org/wiki/KISS_principle) philosophy and only aim to do one thing. There are tools which manage to do the other things better, so make the current one as integrable as possible.

Purrito Bin is very, very easily integrated into any setup. It runs as an unprivileged user in a single directory without access to any other systems resource.

In OpenBSD, it is automatically [pledges](https://man.openbsd.org/pledge) and [unveils](https://man.openbsd.org/unveil) the bare minimum to function, so even in the case of a bug in the code, an attacker has no access to the system.

Pull requests to harden the code by default in linux and other operating systems are highly welcome.

## Extras

### System services

The [services](services/) directory consists of OpenRC and SystemD service files.
They need the `purritobin` user and group to exist.

### Pure C client

[ericonr](https://github.com/ericonr) has made a very nice C client, which also supports encrypted pastes - https://github.com/ericonr/purr-c
It uses [BearSSL](https://www.bearssl.org/) and is very instructive for all who wish to get a small example of using SSL in C together with networking.

## Credits
[uNetworking](https://github.com/uNetworking): for their [uWebSockets](https://github.com/uNetworking/uWebSockets) and [uSockets](https://github.com/uNetworking/uSockets)<br/>
[brix](https://github.com/brix/): for their [crypto-js](https://github.com/brix/crypto-js/)<br/>
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin<br/>
