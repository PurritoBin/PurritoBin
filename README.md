# Purrito Bin  (=♡ᆺ♡=)
[![pipeline](https://github.com/PurritoBin/PurritoBin/workflows/pipeline/badge.svg)](https://github.com/PurritoBin/PurritoBin/actions?query=workflow:pipeline)
[![GitHub release (latest SemVer including pre-releases)](https://img.shields.io/github/v/release/PurritoBin/PurritoBin?include_prereleases)](https://github.com/PurritoBin/PurritoBin/releases)
[![GitHub license](https://img.shields.io/github/license/PurritoBin/PurritoBin.svg)](https://github.com/PurritoBin/PurritoBin/blob/master/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues-raw/PurritoBin/PurritoBin)](https://github.com/PurritoBin/PurritoBin/issues)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/PurritoBin/PurritoBin/issues)

[![Docker Cloud Build Status](https://img.shields.io/docker/cloud/build/purritobin/purritobin)](https://hub.docker.com/r/purritobin/purritobin)
[![Docker Pulls](https://img.shields.io/docker/pulls/purritobin/purritobin)](https://hub.docker.com/r/purritobin/purritobin)
[![Docker Image Size](https://img.shields.io/docker/image-size/purritobin/purritobin/latest)](https://hub.docker.com/r/purritobin/purritobin)


ultra fast, minimalistic, encrypted command line paste-bin

[![Packaging status](https://repology.org/badge/vertical-allrepos/purritobin.svg)](https://repology.org/project/purritobin/versions)

## Features and Highlights

- *Very* lightweight: 2-3 MB of RAM on average.
- Listen on multiple address/port combinations, both IPv4 and IPv6.
- Configurable paste size limit.
- Auto-cleaning of pastes, with configurable paste lifetime at submission time.
   - `domain.tld/{day,week,month}`
   - `domain.tld/<time-in-minutes>`
- Paste storage in plain text, easy to integrate with all web servers (Apache, Nginx, etc.).
- Encrypted pasting similar to [PrivateBin](https://github.com/PrivateBin/PrivateBin).
- Optional **`https`** support for secure communication.
- Tiny code base, less than 1000 lines of code, for very easy auditing.
- Well documented, `man purrito`.

## docker （ฅ＾・ﻌ・＾）ฅ

### Parameters

The docker image allows passing the following variables to configure PurritoBin:

| Variable | Default | Description |
|--------- | ------- | ----------- |
| DOMAIN | `http://localhost:42069/` | **domain** used as prefix of returned paste |
| MAXPASTESIZE  | `65536` | **maximum paste size** allowed, in *BYTES* |
| SLUGSIZE | `7` | **length** of the *randomly generated string id* for the paste |
| TLS | `NO` | enable listening via **`https`** |
| SERVERNAME | `localhost` | **server name indication** used for *TLS* handshakes, must be valid for the given certificates |
| PUBLICKEY | `/etc/purritobin/public.crt` | *TLS* public certificate |
| PRIVATEKEY | `/etc/purritobin/private.crt` | *TLS* private certificate|

### Volumes

It is recommended to mount at least the first two volumes for persistent storage

| Volume | Description |
|------- | ----------- |
| /var/www/purritobin | default location for storing pastes |
| /var/db/purritobin | default location for storing timestamp database |
| /etc/purritobin | default location for loading certificates **if** enabling TLS |

### Examples

For all examples below, remember to substitute the value of `DOMAIN` from `localhost` to the actual domain/IP of the machine.

PurritoBin listens on port 42069 by default.

#### docker cli

##### HTTP

A simple example:
- Run the server while listening for pastes on port `8080`
  - Map host port `8080` to container port `42069`
- Create a persistent store of pastes in host folder `/data/apps/purritobin`
  - Make a shared volume by mounting `/data/apps/purritobin/pastes` to `/var/www/purritobin` inside the container
  - Make another shared volume by mounting `/data/apps/purritobin/database` to `/var/db/purritobin`

```
docker run -d \
  --name=purritobin \
  -e DOMAIN="http://localhost:8080/" \
  -p 8080:42069 \
  -v /data/apps/purritobin/pastes:/var/www/purritobin \
  -v /data/apps/purritobin/database:/var/db/purritobin \
  --restart unless-stopped \
  purritobin/purritobin
```

To do a test paste to the above server
```
  $ echo "cool paste" | curl --silent --data-binary "@${1:-/dev/stdin}" "http://localhost:8080/"
  http://localhost:8080/purr1t0
  $ curl --silent http://localhost:8080/purr1t0
  cool paste
```

##### HTTPS

To run with `https`, the public and private keys need to be provided to the container and mounted at `/etc/purritobin`.
By default, it is assumed that the public and private keys are stored at `/etc/purritobin/public.crt` and `/etc/purritobin/private.crt`, respectively.
For example, assuming that the certificates, for the domain `localhost`, are stored on the host machine at `/data/apps/certificates/{public,private}.crt`, PurritoBin can be started in **`https`** mode with:

```
docker run -d \
  --name=purritobin \
  -e DOMAIN="https://localhost:42069/" \
  -e MAXPASTESIZE=65536 \
  -e SLUGSIZE="7" \
  -e TLS="YES" \
  -e PUBLICKEY="/etc/purritobin/public.crt" \
  -e PRIVATEKEY="/etc/purritobin/private.crt \
  -e SERVERNAME="localhost" \
  -p 8080:42069 \
  -v /data/apps/purritobin/:/var/www/purritobin \
  -v /data/apps/purritobin/database:/var/db/purritobin \
  -v /data/apps/certificates:/etc/purritobin \
  --restart unless-stopped \
  purritobin/purritobin
```

## Manual setup （ฅ＾・ﻌ・＾）ฅ

### Requirements

- [uSockets](https://github.com/uNetworking/uSockets/)
- [uWebSockets](https://github.com/uNetworking/uWebSockets/)
- [lmdbxx](https://github.com/hoytech/lmdbxx)

If these are not available in your OS repositories, you can manually install them by following the steps in the [GitHub workflow](https://github.com/PurritoBin/PurritoBin/actions?query=workflow:pipeline)

### Compilation

```
$ make
$ make install
```

or to install to a different location, use the `DESTDIR` or `PREFIX` variables.

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
Define these functions somewhere in the dot files of your shell (works on all POSIX compliant shells).

```
: ${P_SERVER=bsd.ac}
: ${P_PORT=42069}
: ${P_MAXTIME=30}

# POSIX shell client to upload standard message
purr() {
	curl --silent --max-time "${P_MAXTIME}" --data-binary "@${1:-/dev/stdin}" "${P_SERVER}:${P_PORT}"
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
	curl --max-time "${P_MAXTIME}" --write-out "\n" --silent "${baseurl}/${paste}" | openssl enc -aes-256-cbc -base64 -d -K ${key} -iv ${iv}
	unset url baseurl vals paste key iv
}
```

#### Client usage
You can paste the `stdout` from a command, paste a file or paste while taking
input from a file or from the command line.
```
    ~$ purr c00lfile.txt
    https://bsd.ac/purrit0

    ~$ purr < h0tfile.txt
    https://bsd.ac/purri1o

    ~$ echo Hello world. | purr
    https://bsd.ac/d4nklit

    ~$ purr
    This is my input
    it can span multiple lines
    <Ctrl-d to exit>
    https://bsd.ac/curlpr0
```

### Encrypted Storage Clients  (=｀ᆺ├┬┴┬┴

In a encrypted storage setting, the paste is encrypted before sending it to the server.
Now the server will only be used as a storage bin and even in case of a non-https connection,
you are guaranteed that no one else will be able to read the data that you have sent.

#### How does it work?
 Steps automatically done by the provided clients, on the client side:
 - Randomly generate an ecryption **key** and initialization vector **iv**.
 - Encrypt your **data** using said **key** and **iv**, the encrypted data is called the **cipher**.
 - Send the **cipher** to the pastebin and get a standard paste url as above, which will be converted to the form
 ```
 https://bsd.ac/paste.html#pasteID_key_iv
 ```
The standard client is `meow`, a companion to the above `purr`, which is also POSIX compliant and should work in all shells. It has exactly the same usage as `purr`, so please look at those examples to see how to use it.
The only dependency on the client side is the presence of [LibreSSL](https://www.libressl.org/) or [OpenSSL](https://www.openssl.org/), to do the encryption.

There will be other clients in the [clients](clients/) folder, added on demand for other platforms, which will allow you to do all this automatically.
Pull requests for other clients are highly welcome.

#### Why is this secure?
- All the encryption is done on the client side.
- Only the **cipher** is sent to PurritoBin.
- The url only contains the encrypted **cipher** and has no knowledge of the actual data.
- When you visit the html webpage the **key** is in the **hash property** of the webpage, which is never sent to the server.
- All decryption is done inside the browser, using javascript, [Crypto JS](https://github.com/brix/crypto-js) on the client side.

NOTE: Anyone who has the full link is going to be able to read the decrypted text. Only send the full hash included url to people you trust, through a secure communications channel.


## Design principles
The aim is to follow the [KISS](https://en.wikipedia.org/wiki/KISS_principle) philosophy and only aim to do one thing. There are tools which manage to do the other things better, so make the current one as integrable as possible.

Purrito Bin is very, very easily integrated into any setup. It runs as an unprivileged user in a single directory without access to any other systems resource.

In OpenBSD, it is automatically [pledges](https://man.openbsd.org/pledge) and [unveils](https://man.openbsd.org/unveil) the bare minimum to function, so even in the case of a bug in the code, an attacker has no access to the system.

Pull requests to harden the code by default in linux and other operating systems are highly welcome.

### What PurritoBin provides
- Auto slug generation and returning paste url.
- Efficient limiting of paste size by cutting off requests at threshold, stopping network blockage.
- Auto cleaning of pastes, depending on submission URL.
  - Submit to domain.tld/{day,week,month}
  - or submit to domain.tld/300 - for paste life of 300 minutes
- Submission port for users to submit.
- Tiny server to browse the pastes. It is optimized for small paste sizes. If accepting really large paste, it is recommended to not use this and instead use a dedicated web server, such as [httpd(8)](https://man.openbsd.org/httpd.8), [apache](https://httpd.apache.org/), [nginx](https://www.nginx.com/) or literally any other web server.
- You can run it on an internal system so that it is accessible only by the people inside the network.

### What PurritoBin does NOT provide
- Request throttling
  - Use a firewall, like [pf](https://www.openbsd.org/faq/pf/filter.html), [nftables](https://wiki.nftables.org/wiki-nftables/index.php/Main_Page) or **(god forbid)** [iptables](https://linux.die.net/man/8/iptables), to manage this, they are designed for exactly this kind of feature.

### Extras

#### System services

The [services](services/) directory consists of OpenRC and SystemD service files.
They need the `purritobin` user and group to exist.

#### Pure C client

[ericonr](https://github.com/ericonr) has made a very nice C client, which also supports encrypted pastes - https://github.com/ericonr/purr-c
It uses [BearSSL](https://www.bearssl.org/) and is very instructive for all who wish to get a small example of using SSL in C together with networking.

## Credits
[uNetworking](https://github.com/uNetworking): for their [uWebSockets](https://github.com/uNetworking/uWebSockets) and [uSockets](https://github.com/uNetworking/uSockets)<br/>
[brix](https://github.com/brix/): for their [crypto-js](https://github.com/brix/crypto-js/)<br/>
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin<br/>
