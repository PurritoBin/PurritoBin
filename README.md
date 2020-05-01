# Purrito Bin (=♡ᆺ♡=)

ultra fast, minimalistic, command line paste-bin

## Client (=｀ﻌ´=)

### Basic client
Define this function somewhere in the dot files of your shell (works on all POSIX shells)

```
purr() {
	curl --silent --data-binary "@${1:-/dev/stdin}" bsd.ac:42069;
}
```

NOTE (specific to bsd.ac):
- One paste every 3 seconds, abusers will be automatically banned for 10 minutes
- Paste size limited to 64KB (will be truncated)
- Connection is not encrypted, use [zero-knowledge client](#zero) if you need full security

#### Client usage
You can paste the stdout from a command, paste a file or paste while taking 
input from a file or from the command line
```
~$ echo Hello world. | purr
https://bsd.ac/d4nklit

~$ purr dankfile.txt
https://bsd.ac/noobm4x

~$ purr < litfile.txt
https://bsd.ac/pron00b

~$ purr
This is my input
it can span multiple lines
<Ctrl-d>
https://bsd.ac/curlpr0
```

### Zero Knowledge Storage Clients (Work in Progress)  (=｀ᆺ├┬┴┬┴

In a zero knowledge storage setting, the paste is encrypted before sending it to the server.  
Now the server will only be used as a storage bin and even in case of a non-https connection, 
you are guaranteed that no one else will be able to read

#### How does it work?
 Client side:
 - Encrypt the *paste* using some *key*, the encrypted text is called the *cipher*
 - Send the *cipher* to the pastebin, using any of the encryption clients (in the clients directory) and get a paste url 
 ```
 https://bsd.ac/paste.html#pasteID_encryptionKEY
 ```

There are clients in the [clients](clients/) folder which allow you to do all this automatically, including POSIX compliant shell clients 

### Why is this secure?
- The html webpage only contains the encrypted *cipher* and has no knowledge of the *paste*
- When you visit the html webpage the *key* is in the hash property of the webpage, which is never sent to the server
- All decryption is done inside the browser on the client side



## Server （ฅ＾・ﻌ・＾）ฅ

## Requirements

- [uWebSockets](https://github.com/uNetworking/uWebSockets/)

### Installation

Clone the repository and compile

```
make
./purrito [ options ]
```

To install the program

```
make
make install
```

### Usage

```
 usage: purrito [-hdsipmg]                                       
        -h                                                      
            print this help                                   
	    
        -d domain                                               
            REQUIRED                                            
            domain that will be used as prefix of returned paste
            NOTE: should be the full name, including trailing / 
              e.g. https://bsd.ac/                              

        -s storage_directory                                    
            REQUIRED                                            
            path to the storage directory for storing the paste 
            NOTE: should exist prior to creation and should be  
                  writable by the user running purrito          
              e.g. /var/www/purrito                             

        -i bind_ip                                              
            DEFAULT: 0.0.0.0                                    
            IP on which to listen for incoming connections      

        -p bind_port                                            
            DEFAULT: 42069                                      
            port on which to listen for connections             

        -m max_paste_size (in bytes)                            
            DEFAULT: 65536 (64KB)                               

        -g slug_size                                            
            DEFAULT: 7                                          
```


## Credits
[uNetworking](https://github.com/uNetworking): for their [uWebSockets](https://github.com/uNetworking/uWebSockets)  
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin

