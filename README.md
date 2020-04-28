# Purrito Bin

minimalistic command line paste-bin

## Client

Define this function somewhere in the dot files of your shell (works on all POSIX shells)

```
purr() {
	nc bsd.ac 42069 < ${1:-/dev/stdin}
}
```
NOTE: `nc` is the command for [netcat](https://en.wikipedia.org/wiki/Netcat), replace `nc` with the equivalent command for netcat in your system

### Client usage
```
~$ echo Hello world. | nc bsd.ac 42069
Your paste is available at: https://bsd.ac/d4nkl1t

~$ echo Bye world. | purr
Your paste is available at: https://bsd.ac/n0td4nk

~$ purr d4nkf1l3.txt
Your paste is available at: https://bsd.ac/n00bm4x

~$ purr < l1tf1l3.txt
Your paste is available at: https://bsd.ac/pr0n00b
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
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin
