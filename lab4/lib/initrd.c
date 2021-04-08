#include "../include/gpio.h"
#include "../include/uart.h"
#include "../include/stringUtils.h"
#include "../include/shell.h"

/* cpio hpodc format */
typedef struct {
    char magic[6]; //070701
    char ino[8];  //inodes number from disk
    char mode[8]; //specifies both the regular permissions and file type
    char uid[8]; //userid
    char gid[8]; //groupid
    char nlink[8];  //number of links to this file
    char mtime[8];  //modification time
    char filesize[8];
    char devmajor[8];
    char devminor[8];
    char rdevmajor[8];
    char rdevminor[8];
    char namesize[8];
    char check[8]; //always set to 0 by writers and ignored by reader
} __attribute__((packed)) cpio_t; // tell compiler don't do the alignment in order to save memory space
/*
    convert hexadecimal string into decimal
 */
unsigned long hexToDex(char *s){
    unsigned long r = 0;
    for(int i = 0 ;i < 8 ; ++i){
        if(s[i] >= '0' && s[i] <= '9'){
            r = r* 16 + s[i]-'0';
        }else{
            r = r * 16 + s[i]-'a'+10;
        }
    }
    return r;
}
/*
 align to multiple of 4
 */
void Align_4(void* size){
    unsigned long* x =(unsigned long*) size;
    if((*x)&3){
        (*x) += 4-((*x)&3);
    }
}
/*
 print file content
 */

static void printFilecontent(cpio_t *addr){
    unsigned long psize = hexToDex(addr->namesize), dsize=hexToDex(addr->filesize);
    unsigned long HPP = sizeof(cpio_t) + psize;
    Align_4(&HPP);
    Align_4(&dsize);

    char *data = (char*)((char*)addr + HPP);
    uart_puts("\n");
    uart_puts("---------contents----------\n");
    for(int i = 0; i < dsize ; ++i){
        uart_send(data[i]);
    }
    uart_puts("---------------------------\n");


}
cpio_t* findFile(cpio_t* addr, char* str){
    while(1){
        unsigned long psize=hexToDex(addr->namesize),dsize=hexToDex(addr->filesize);
        unsigned long HPP = sizeof(cpio_t) + psize;
        Align_4(&HPP);
        Align_4(&dsize);

        char *data = (char*)((char*)addr + HPP);
        if(compString((char*)(addr+1), "TRAILER!!!") == 0) break;
        if(compString((char*)(addr+1), str) == 0) return addr;
        addr=(cpio_t*)(data+dsize);
    }

    return 0;
}


void getName(char* target){
    uart_puts("Please enter file name: ");
    read_input(target);
}
/**
 * List the contents of an archive
 */
void cpio()
{
    uart_puts("Size\t\tFilename\n");
    cpio_t* addr = (cpio_t*)0x20000000;
    // iterate on archive's contents
    // if it's a cpio archive. Cpio also has a trailer entry
    while(compString((char*)(addr+1),"TRAILER!!!")!=0) {
        unsigned long psize=hexToDex(addr->namesize),dsize=hexToDex(addr->filesize);
        unsigned long HPP = sizeof(cpio_t) + psize;

        Align_4(&HPP);
        Align_4(&dsize);

        char *data = (char*)((char*)addr + HPP);
        // print out meta information
        uart_hex(dsize);      // file size in hex
        uart_puts("\t");
        uart_puts((char *)(addr+1));      // filename
        uart_puts("\n");
        // jump to the next file
        addr=(cpio_t*)(data+dsize);
    }

    addr = (cpio_t*)0x20000000;
    char target[100];
    getName(target);
    cpio_t* entry = findFile(addr,target);
    if(entry){
        printFilecontent(entry);
    }else{
        uart_puts("no such file\n");
    }
}


