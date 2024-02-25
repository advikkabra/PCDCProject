#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define xmalloc(size)                                                           \
  ({                                                                           \
    void *ptr = malloc(size);                                                  \
    if (ptr == NULL) {                                                         \
      fprintf(stderr, "Memory allocation failed\n");                           \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    ptr;                                                                       \
  })

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
#define TABLE_MAX_PAGES 100

// define the following for making it easier to use (optional):
// const uint32_t:
// ID_SIZE, USERNAME_SIZE, EMAIL_SIZE
// ID_OFFSET, USERNAME_OFFSET, EMAIL_OFFSET
// ROW_SIZE

const uint32_t ID_SIZE = sizeof(uint32_t);
const uint32_t ROW_SIZE = 512;
const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;

// necessary struct defenitions

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;
typedef enum { SUCCESS, INVALID, SYNTAX_ERROR, TABLE_FULL } ResponseCode;

typedef struct {
  char *buffer;
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
  void *pages[TABLE_MAX_PAGES];
} Table;

typedef struct {
  StatementType type;
  Row row_to_insert;
} Statement;

static inline void print_prompt() { printf("db > "); }

void *row_slot(Table *table, uint32_t row_num) {
  uint32_t page_num = row_num / ROWS_PER_PAGE;
  void *page = table->pages[page_num];
  if (page == NULL) {
    // Allocate memory only when we try to access page
    page = table->pages[page_num] = xmalloc(PAGE_SIZE);
  }
  uint32_t row_offset = row_num % ROWS_PER_PAGE;
  uint32_t byte_offset = row_offset * ROW_SIZE;
  return page + byte_offset;
}

InputBuffer *new_input_buffer() {
  // allocate memory, initialize other members of the struct

  char *buffer = (char *)xmalloc(ROW_SIZE);
  InputBuffer *input_buffer = (InputBuffer *)xmalloc(sizeof(InputBuffer));

  input_buffer->buffer = buffer;
  input_buffer->buffer_length = ROW_SIZE;
  input_buffer->input_length = 0;

  return input_buffer;
}

Table *new_table() {
  // allocate memory, initialize other memebers
  // of the struct and each page to NULL

  Table *table = (Table *)xmalloc(sizeof(Table));
  table->num_rows = 0;

  for (int i = 0; i < TABLE_MAX_PAGES; i++)
    table->pages[i] = NULL;

  return table;
}

//--(A)
void read_input(InputBuffer *input_buffer) {
  // takes the input via getline into the buffer

  int charlen =
      getline(&input_buffer->buffer, &input_buffer->buffer_length, stdin);
  charlen--;
  if (!charlen) {
    printf("Error reading input! Did you type anything?\n");
    return;
  }

  input_buffer->buffer[charlen] = '\0';
  input_buffer->input_length = charlen;
}

//--(B) change the return type to indicate success or invalid command
ResponseCode do_meta_command(InputBuffer *input_buffer) {
  // in .exit case close_input_buffer:
  // free buffer and struct memory

  if (strncmp(input_buffer->buffer + 1, "exit", 5) == 0) {
    free(input_buffer->buffer);
    free(input_buffer);
    printf("Bye bye!\n");
    return SUCCESS;
  }
  return INVALID;
}

//--(C) change return type from void to some flag to denote success or invalid
// statement
ResponseCode prepare_statement(InputBuffer *input_buffer,
                               Statement *statement) {
  // if insert, then set statement type based
  //  on enum values defined above,
  //  Initialize row_to_insert struct's values based
  //  on the next 3 arguments,
  // return a success flag
  //  if <3 args, give syntax error

  // if select, then set statement type based
  //  on enum values defined above,
  //  return success flag
  //

  char *cursor;
  char *first_word = strtok_r(input_buffer->buffer, " ", &cursor);

  if (strncmp(first_word, "insert", 7) == 0) {
    statement->type = STATEMENT_INSERT;

    Row row = {};
    char *id_s = strtok_r(cursor, " ", &cursor);
    char *username = strtok_r(cursor, " ", &cursor);
    char *email = strtok_r(cursor, " ", &cursor);

    if (!(id_s && username && email))
      return SYNTAX_ERROR;

    // Check for invalid id
    for (char *c = id_s; *c != '\0'; c++)
      if (*c > '9' || *c < '0') {
        printf("ID is not numeric!\n");
        return SYNTAX_ERROR;
      } else if (c - id_s > 8) {
        printf("ID too long!\n");
        return SYNTAX_ERROR;
      }

    // Check for invalid username
    if (strnlen(username, COLUMN_USERNAME_SIZE + 1) > COLUMN_USERNAME_SIZE) {
      printf("Username too long!\n");
      return SYNTAX_ERROR;
    }

    // Check for invalid email
    if (strnlen(email, COLUMN_EMAIL_SIZE + 1) > COLUMN_EMAIL_SIZE) {
      printf("Email too long!\n");
      return SYNTAX_ERROR;
    }

    int id = atoi(id_s);

    row.id = id;
    strncpy(row.username, username, COLUMN_USERNAME_SIZE);
    strncpy(row.email, email, COLUMN_EMAIL_SIZE);

    statement->row_to_insert = row;

    return SUCCESS;
  } else if (strncmp(first_word, "select", 7) == 0) {
    statement->type = STATEMENT_SELECT;

    return SUCCESS;
  }

  // If neither, return invalid command flag
  return INVALID;
}

void serialize_row(Row *source, void *destination) {
  memcpy(destination, &(source->id), ID_SIZE);
  memcpy(destination + ID_SIZE, (source->username), COLUMN_USERNAME_SIZE);
  memcpy(destination + ID_SIZE + COLUMN_USERNAME_SIZE, (source->email),
         COLUMN_EMAIL_SIZE);
}

void deserialize_row(void *source, Row *destination) {
  memcpy(&(destination->id), source, ID_SIZE);
  memcpy((destination->username), source + ID_SIZE, COLUMN_USERNAME_SIZE);
  memcpy((destination->email), source + ID_SIZE + COLUMN_USERNAME_SIZE,
         COLUMN_EMAIL_SIZE);
}

//--(D) change return type to indicate success or table full error
ResponseCode execute_statement(Statement *statement, Table *table) {

  // execute insert:
  if (statement->type == STATEMENT_INSERT) {

    if (table->num_rows == TABLE_MAX_ROWS)
      return TABLE_FULL;

    Row *row_to_insert = &(statement->row_to_insert);
    serialize_row(row_to_insert, row_slot(table, table->num_rows));
    table->num_rows++;
    return SUCCESS;
  }
  // execute select:
  else if (statement->type == STATEMENT_SELECT) {
    //  row is a local variable here
    printf("   ID | %32s | %255s\n", "USERNAME", "EMAIL");
    for (int i = 0; i < table->num_rows; i++) {
      Row row;
      deserialize_row(row_slot(table, i), &row);
      printf(" %4d | %32s | %255s\n", row.id, row.username, row.email);
    }
    return SUCCESS;
  }
}

void free_table(Table *table) {
  for (int i = 0; table->pages[i]; i++) {
    free(table->pages[i]);
  }
  free(table);
}

int main(int argc, char *argv[]) {
  Table *table = new_table();
  InputBuffer *input_buffer = new_input_buffer();
  while (true) {
    print_prompt(); // like "db >"

    // call a read_input fxn --(A)
    read_input(input_buffer);

    // commands available on database:
    //  main commands->
    // insert: insert row of the form id,name,email,
    //  select: print all rows
    // meta command->
    //.exit: return from the program
    //  The current implementation does not
    //  store the records in another file (in a retrievable form)
    //  That will be implemented in task-2
    //  i.e, once you .exit, data is lost

    if (input_buffer->buffer[0] == '.') {
      // checking for meta_command
      // call do_meta_command(input_buffer) --(B)
      ResponseCode response = do_meta_command(input_buffer);

      // handle invalid meta_command case
      if (response == INVALID) {
        printf("Invalid command, did you mean .exit?\n");
        continue;
      } else {
        // Do all exit work here
        free_table(table);
        return 0;
      }
    }

    Statement statement;

    // call prepare_statement fxn --(C)
    // If this returns invalid/syntax error, output error message
    // else if it  returns success,

    ResponseCode response = prepare_statement(input_buffer, &statement);

    if (response == INVALID) {
      printf("Invalid command, did you mean insert or select?\n");
      continue;
    } else if (response == SYNTAX_ERROR) {
      printf("Syntax error! Did you give enough arguments for a row [id, "
             "username, email]\n");
      continue;
    }

    // call execute_statement fxn --(D)

    response = execute_statement(&statement, table);
    // if success, print "executed",
    if (response == SUCCESS)
      printf("Executed.\n");
    // else print table full error
    else if (response == TABLE_FULL)
      printf("Table full!\n");
  }
}

// EXTRA:
//  Check for negative index
//  Check for username length before filling it into the row
//  read about  strtok() and implement (optional)
