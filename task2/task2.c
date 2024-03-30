#include "task2.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

void print_prompt() { printf("db > "); }

// Struct defns given in Task2.h
// Function declarations are also given in Task2.h
// You need not use the same fxns,mo dify according to your implementation
// and edit the declarations in .h later
// You can add/delete things from .h file according to your code

// add all fxns from task-1 here

// add pager fxns with the functionalities given in Pager_template

Pager *pager_open(const char *filename){
    int fd = open(filename,O_RDWR |      // Read/Write mode
                    O_CREAT,  // Create file if it does not exist
                S_IWUSR |     // User write permission
                    S_IRUSR   // User read permission
);
    if (fd == -1) {
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }

    Pager* pager = (Pager*)xmalloc(sizeof(Pager));
    pager->file_descriptor = fd;
    for (int i=0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }
    
    return pager;
};

void pager_flush(Pager *pager, uint32_t page_num, uint32_t size){
    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
    write(pager->file_descriptor, pager->pages[page_num], size);
}
// add cursor fxns with the functionalities given in Cursor_template

int main(int argc, char *argv[]) {
}
