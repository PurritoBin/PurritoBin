# Purrito Bin

ultra fast, minimalistic, command line paste-bin

## Client

Define this function somewhere in the dot files of your shell (works on all POSIX shells)

```
purr() {
	curl --data-binary "@${1:-/dev/stdin}" bsd.ac:42069;
}
```

NOTE (specific to bsd.ac):
- One paste every 3 seconds, abusers will be automatically banned for 10 minutes
- Paste size limited to 64KB (will be truncated)
- Connection is not encrypted, use [zero-knowledge client](#zero) if you need full security

### Client usage
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

## Server

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

