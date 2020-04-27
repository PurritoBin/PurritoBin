# Purrito Bin

minimalistic command line paste-bin

## Usage

```
 usage: purrito [-dsipm]                                         
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
```

## Installation

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

## Credits
[solusipse](https://github.com/solusipse): for their [fiche](https://github.com/solusipse/fiche/) pastebin