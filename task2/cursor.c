#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include <task2.h>

//cursor abstraction

Cursor* table_start(Table* table){
    Cursor* cursor = (Cursor* )xmalloc(sizeof(Cursor));
    
    cursor->table = table;
    cursor->row_num = 0;
    cursor->end_of_table = false;

    return cursor;
} 


Cursor* table_end(Table* table) {
    Cursor* cursor = (Cursor* )xmalloc(sizeof(Cursor));

    cursor->table = table;
    cursor->row_num = table->num_rows ;
    cursor->end_of_table = true;

}
//creates new cursors

// Now, row_slot() fxn renamed to cursor_location()
// modify implementation accordingly

void* cursor_value(Cursor* cursor){

    uint32_t page_num = cursor->row_num / ROWS_PER_PAGE;
    void* page = get_page(cursor->table->pager, page_num);
    if(page == NULL){
        fprintf(stderr,"Error! Unable to access page.");
    }

    uint32_t row_offset = cursor->row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

// next row
void cursor_advance(Cursor* cursor)
{
    if(cursor->end_of_table == true){
        fprintf(stderr,"Index Error! Accessing row out of table.");
        exit(EXIT_FAILURE);
    }
     
    if(cursor->row_num == cursor->table->num_rows){
        cursor->end_of_table = true;
    } else {
        cursor->row_num++;
    }
}   

// use the above cursor fxns (instead of rows) in
// insert (table_end, etc.)
// select (table_begin, aursor_advance, etc.)
