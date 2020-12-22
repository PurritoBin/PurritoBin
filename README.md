# Purrito Bin  (=♡ᆺ♡=)  
[![GitHub release (latest SemVer including pre-releases)](https://img.shields.io/github/v/release/PurritoBin/PurritoBin?include_prereleases)](https://github.com/PurritoBin/PurritoBin/releases)
[![GitHub license](https://img.shields.io/github/license/PurritoBin/PurritoBin.svg)](https://github.com/PurritoBin/PurritoBin/blob/master/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues-raw/PurritoBin/PurritoBin)](https://github.com/PurritoBin/PurritoBin/issues)
[![contributions welcome](https://img.shields.io/badge/contributions-welcome-brightgreen.svg?style=flat)](https://github.com/PurritoBin/PurritoBin/issues)

Ultra-fast, minimalistic, and encrypted command line paste-bin.

[![Packaging status](https://repology.org/badge/vertical-allrepos/purritobin.svg)](https://repology.org/project/purritobin/versions)

## Client  (=｀ﻌ´=)
### Shell client definitions
Define these functions somewhere in your POSIX-compliant shell dot-files:
```
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
```

#### Client usage
You can paste the `stdout` from a command, paste a file, or paste while taking input from a file or from the CLI:
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

##### LIMITS (specific to bsd.ac):
- One paste every 3 seconds — abusers will be automatically banned for 10 minutes;
- Paste size is limited to 64KB (larger pastes will be aborted); and
- Even though SSL is supported for pasting connections, the server on bsd.ac does not use SSL capabilities because the author is an airhead.

### Encrypted Storage Clients  (=｀ᆺ├┬┴┬┴
With encrypted storage, the paste is encrypted before uploading to the server.

Said-server will only ever be used as a storage bin. You are guaranteed that only you and those you choose will be able to read the data that you have sent, even with unencrypted (non-HTTPS) connections.

#### How does it work?
These steps are automatically done by the provided client-side applications:
1. Randomly generate an encryption **key**;
2. Encrypt your **data** using said-**key**, creating a **cipher**; and
3. Send the **cipher** to PurritoBin and receive a standard paste URL converted to the following form:
 ```
 https://bsd.ac/paste.html#pasteID_encryptionKEY
 ```
The standard client is `meow`, an also-POSIX-compliant companion to the above `purr`. It has exactly the same usage as `purr`, so please refer to those examples to see how to use it.

The only dependency on the client side is the presence of [LibreSSL](https://www.libressl.org/) or [OpenSSL](https://www.openssl.org/) for encryption.

There will be other clients in the [clients](clients/) folder, added per-demand for other platforms.  
Pull requests for other clients are highly welcome.

### Why is this secure?
- All the encryption is done on the client side;
- Only the **cipher** is sent to PurritoBin;
- The URL only contains the encrypted **cipher** and has no knowledge of the actual data;
- When you visit the HTML link the **key** is in the **hash property** of the webpage, which is never sent to the server; and
- All decryption is done inside the browser, using JavaScript — [Crypto JS](https://github.com/brix/crypto-js) on the client side.

#### NOTE:
Anyone who has the full link is able to read the decrypted text. Only send the full hash URL to people you trust through a secure communications channel.

## Server （ฅ＾・ﻌ・＾）ฅ

### Design principles
The aim is to follow the [KISS](https://en.wikipedia.org/wiki/KISS_principle) philosophy to do one thing and do it well (there are tools which manage to do other things better, so make the current tool as integrable as possible).

Purrito Bin is very, very easily integrated into any setup. It runs as an unprivileged user in a single directory without additional system access.

In OpenBSD, it automatically [pledges](https://man.openbsd.org/pledge) and [unveils](https://man.openbsd.org/unveil) the bare minimum to function. Even in the case of a bug in the code, an attacker has no access to the system.

Pull requests to harden the code by default in other Unix-like operating systems are highly welcome.

### What PurritoBin provides
- Auto-slug generation and returned paste URL;
- Efficient limiting of paste size by cutting off requests at threshold, stopping network blockage;
- Submission port for users to submit to; and
- You can run it on an internal system so that it is only accessible to the people inside the intranet.

### What PurritoBin does NOT provide
- Server to browse the pastes;
  - The pastes are plain text files, so they can be shown using [httpd(8)](https://man.openbsd.org/httpd.8), [apache](https://httpd.apache.org/), [nginx](https://www.nginx.com/) or literally any other web server. Use them — they are much better at this and can implement a lot of other useful functionality.
- Auto cleaning of pastes; or
  - Use a [cron](https://en.wikipedia.org/wiki/Cron) job to manage this. It is a lot more efficient and also gives you more control.
- Request throttling.
  - Use a firewall, like [pf](https://www.openbsd.org/faq/pf/filter.html), [nftables](https://wiki.nftables.org/wiki-nftables/index.php/Main_Page) or **(god forbid)** [iptables](https://linux.die.net/man/8/iptables), to manage this, they are designed for exactly this kind of feature.

### Requirements
- [uSockets](https://github.com/uNetworking/uSockets/)
- [uWebSockets](https://github.com/uNetworking/uWebSockets/)

### Installation
```
$ make
$ make install
```
To install to a different location, use the `DESTDIR` variable.

### Usage
The server is executed using the command `purrito`. To quickly view the available options:
```
$ purrito -h
usage: purrito [-cdeghiklmnpsw] -d domain [-c public_cert_file]
               [-e dhparams_file] [-g slug_size] [-h] [-i bind_ip]
               [-k private_key_file] [-l] [-m max_paste_size] [-n server name]
               [-p bind_port] [-s storage_directory] [-w passphrase]
```
For a more in-depth explanation, refer to the provided man page:
```
$ man purrito
```

### Running the encrypted PurritoBin
Make sure that provided link to the `-d ` domain option is also able to serve the `paste.html` from this repository.

## Workarounds for older compilers
When building with older compilers, you also need to provide the `stdc++fs` library, which can be done by compiling using:
```
make LDFLAGS=-lstdc++fs
```

### Extras
#### Pure C client
[ericonr](https://github.com/ericonr) has made a very nice C client, which also supports encrypted pastes — https://github.com/ericonr/purr-c   
It uses [BearSSL](https://www.bearssl.org/) and is very instructive for all who wish to get a small example of using SSL in C together with networking.

## Credits
[uNetworking](https://github.com/uNetworking): for their [uWebSockets](https://github.com/uNetworking/uWebSockets) and [uSockets](https://github.com/uNetworking/uSockets)
[brix](https://github.com/brix/): for their [crypto-js](https://github.com/brix/crypto-js/)  
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin

