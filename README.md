# DNS Tunneling Tool (Hodnotenie: 21/20b)

### *For more detailed information check official documentation `manual.pdf` located in root*

###  Author
Andrej Pavlovič <xpavlo14@vutbr.cz>

### Created
9.11.2022

### Files
manual.pdf  - documentation

Makefile    - compilation file

README.md   - this readme

src/        - source files

test.sh     - testing script

### About program
This is tool for DNS tunneling implementing both client and server.

### Compile 
**make** from root dir

### Run example
**dns_receiver example.com received/**

**dns_sender -u 127.0.0.1 -s 0 example.com receive.txt ./send.txt**

For help run them without parameters.
