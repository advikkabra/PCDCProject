#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

//necessary struct defenitions
typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;


typedef struct {
  uint32_t id;
 char username[COLUMN_USERNAME_SIZE];
  char email[COLUMN_EMAIL_SIZE];
} Row;


typedef struct {
  uint32_t num_rows;
  void* pages[TABLE_MAX_PAGES];
} Table;


typedef struct {
  StatementType type;
  Row row_to_insert; 
} Statement;



typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

// define the following for making it easier to use (optional):
//const uint32_t:
// ID_SIZE, USERNAME_SIZE, EMAIL_SIZE
// ID_OFFSET, USERNAME_OFFSET, EMAIL_OFFSET
// ROW_SIZE

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES; 

void* row_slot(Table* table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void* page = table->pages[page_num];
  if (page == NULL) {
    // Allocate memory only when we try to access page
    page = table->pages[page_num] = malloc(PAGE_SIZE);
  }
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}




InputBuffer* new_input_buffer() {
  //allocate memory, initialize other members of the struct
}

 Table* new_table() {
  //allocate memory, initialize other memebers 
  //of the struct and each page to NULL
}



//--(A)
void read_input(InputBuffer* input_buffer) {
 //takes the input via getline into the buffer 
 //Note: check input size and buffer size while using getline
 // If No input-> print error message
 // Initialize input_buffer struct values accordingly
 // Rewrite the newline char to something else
}

//--(B) change the return type to indicate success or invalid command
void do_meta_command(InputBuffer* input_buffer) {
  //in .exit case close_input_buffer: 
  //free buffer and struct memory
}

//--(C) change return type from void to some flag to denote success or invalid statement
void prepare_statement(InputBuffer* input_buffer,Statement* statement) {
  //if insert, then set statement type based
  // on enum values defined above, 
  // Initialize row_to_insert struct's values based 
  // on the next 3 arguments, 
  //return a success flag
  // if <3 args, give syntax error

  //if select, then set statement type based
  // on enum values defined above, 
  // return success flag
  
  // If neither, return invalid command flag

}


void serialize_row(Row* source, void* destination) {
  memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
  // similarly, for username,email
}

void deserialize_row(void* source, Row* destination) {
  memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
// similarly, for username, email
}

//--(D) change return type to indicate success or table full error
void execute_statement(Statement* statement, Table* table) {
 
 //execute insert: 
 // if table's no.of rows >=capacity return table_full flag

 // otherwise,
  Row* row_to_insert = &(statement->row_to_insert);
  serialize_row(row_to_insert, row_slot(table, table->num_rows));

//execute select: 
// row is a local variable here
 deserialize_row(row_slot(table, i), &row);
 // print each row 

 }



void free_table(Table* table) {
    for (int i = 0; table->pages[i]; i++) {
free(table->pages[i]);
    }
    free(table);
}

int main(int argc, char* argv[]) {
      Table* table = new_table();
  InputBuffer* input_buffer = new_input_buffer();
  while (true) {
    print_prompt(); //like "db >"

   //call a read_input fxn --(A)

   //commands available on database:
   // main commands-> 
   //insert: insert row of the form id,name,email,
   // select: print all rows
   //meta command->
   //.exit: return from the program
   // The current implementation does not
   // store the records in another file (in a retrievable form)
   // That will be implemented in task-2
   // i.e, once you .exit, data is lost


   if (input_buffer->buffer[0] == '.') {
     //checking for meta_command
// call do_meta_command(input_buffer) --(B)
     
     //handle invalid meta_command case either internally
     // in do_meta_command OR based on its return value
     // (which can be an enum value or int)
     // just print "unrecognized command"
    }

    Statement statement;

    // call prepare_statement fxn --(C)
    //If this returns invalid/syntax error, output error message
    // else if it  returns success,


    // call execute_statement fxn --(D)
    // if success, print "executed",
    // else print table full error
  }
}


//EXTRA:
// Check for negative index  
// Check for username length before filling it into the row
// read about  strtok() and implement (optional) 
