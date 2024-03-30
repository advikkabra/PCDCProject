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

void *get_page(Pager *pager, uint32_t page_num) {
  if (page_num > TABLE_MAX_PAGES)
    return NULL;
  if (pager->pages[page_num] == NULL) {
    pager->pages[page_num] = xcalloc(ROWS_PER_PAGE, ROW_SIZE);
    ssize_t response = read(pager->file_descriptor,
        pager->pages[page_num], PAGE_SIZE);
    if (response == -1)
      return NULL;
    return pager->pages[page_num];
  } else {
    return pager->pages[page_num];
  }
}

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

    return cursor;
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

PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement){
    char *cursor;
    strtok_r(input_buffer->buffer, " ", &cursor);
    statement->type = STATEMENT_INSERT;

    Row row = {};
    char *id_s = strtok_r(cursor, " ", &cursor);
    char *username = strtok_r(cursor, " ", &cursor);
    char *email = strtok_r(cursor, " ", &cursor);

    if (!(id_s && username && email))
      return PREPARE_SYNTAX_ERROR;

    // Check for invalid id
    for (char *c = id_s; *c != '\0'; c++)
      if (*c > '9' || *c < '0') {
        printf("ID is not numeric!\n");
        return PREPARE_SYNTAX_ERROR;
      } else if (c - id_s > 8) {
        printf("ID too long!\n");
        return PREPARE_STRING_TOO_LONG;
      }

    // Check for invalid username
    if (strnlen(username, COLUMN_USERNAME_SIZE + 1) > COLUMN_USERNAME_SIZE) {
      printf("Username too long!\n");
      return PREPARE_STRING_TOO_LONG;
    }

    // Check for invalid email
    if (strnlen(email, COLUMN_EMAIL_SIZE + 1) > COLUMN_EMAIL_SIZE) {
      printf("Email too long!\n");
      return PREPARE_STRING_TOO_LONG;
    }

    int id = atoi(id_s);
    if(id < 0)
        return PREPARE_NEGATIVE_ID;

    row.id = id;
    strncpy(row.username, username, COLUMN_USERNAME_SIZE);
    strncpy(row.email, email, COLUMN_EMAIL_SIZE);

    statement->row_to_insert = row;

    return PREPARE_SUCCESS;
}
PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement){
  char *cursor;
  char *first_word = strtok_r(input_buffer->buffer, " ", &cursor);

  if (strncmp(first_word, "insert", 7) == 0) {
    return prepare_insert(input_buffer, statement);
  } else if (strncmp(first_word, "select", 7) == 0) {
    statement->type = STATEMENT_SELECT;

    return PREPARE_SUCCESS;
  }

  // If neither, return invalid command flag
  return PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement *statement, Table *table){
    if (table->num_rows != TABLE_MAX_ROWS){
        table->num_rows++;
        void* page = get_page(table->pager, table->num_rows / ROWS_PER_PAGE);
        uint32_t row_offset = table->num_rows % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * ROW_SIZE;

        Row *row_to_insert = &(statement->row_to_insert);
        serialize_row(row_to_insert, page+byte_offset);
        return EXECUTE_SUCCESS;

    } else {
        fprintf(stderr, "Max Table Limit Reached");
        return EXECUTE_TABLE_FULL;
    }
}


ExecuteResult execute_select(Statement *statement, Table *table){
    Cursor *cursor = table_start(table);

    printf("   ID | %32s | %255s\n", "USERNAME", "EMAIL");
    do {
        Row row;
        deserialize_row(cursor_value(cursor), &row);
        printf(" %4d | %32s | %255s\n", row.id, row.username, row.email);
        cursor_advance(cursor);
    }
    while (!cursor->end_of_table);

    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table){
    ExecuteResult status;

    switch (statement->type)
    {
    case STATEMENT_INSERT:
        status = execute_insert(statement, table);
        break;
    
    case STATEMENT_SELECT:
        status = execute_select(statement, table);
        break;
    
    default:
        fprintf(stderr, "Huh? Not supposed to be here.. F");
        break;
    }

    return status;
}

// use the above cursor fxns (instead of rows) in

int main(int argc, char *argv[]) {
}
