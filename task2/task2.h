#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>


#define xmalloc(size)                                                          \
  ({                                                                           \
    void *ptr = malloc(size);                                                  \
    if (ptr == NULL) {                                                         \
      fprintf(stderr, "Memory allocation failed\n");                           \
      exit(EXIT_FAILURE);                                                      \
    }                                                                          \
    ptr;                                                                       \
  })

#define xcalloc(number, size)                                                  \
  ({                                                                           \
    void *ptr = calloc(number, size);                                          \
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

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;
typedef enum { SUCCESS, INVALID, SYNTAX_ERROR, TABLE_FULL } ResponseCode;


#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
    StatementType type;
    Row row_to_insert; // only used by insert statement
} Statement;

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum { EXECUTE_SUCCESS, EXECUTE_TABLE_FULL } ExecuteResult;

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)

void serialize_row(Row *source, void *destination);
void deserialize_row(void *source, Row *destination);

typedef struct {
    int file_descriptor;
    uint32_t file_length;
    void *pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
    uint32_t num_rows;
    Pager *pager;
} Table;

void *get_page(Pager *pager, uint32_t page_num);

typedef struct {
    Table *table;
    uint32_t row_num;
    bool end_of_table; // Indicates a position one past the last element
} Cursor;

Cursor *table_start(Table *table);
Cursor *table_end(Table *table);
void *cursor_value(Cursor *cursor);
void cursor_advance(Cursor *cursor);
void pager_flush(Pager *pager, uint32_t page_num, uint32_t size);
void db_close(Table *table);
MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table);
PrepareResult prepare_insert(InputBuffer *input_buffer, Statement *statement);
PrepareResult prepare_statement(InputBuffer *input_buffer,
                                Statement *statement);
Pager *pager_open(const char *filename);
Table *db_open(const char *filename);
void print_row(Row *row);
ExecuteResult execute_insert(Statement *statement, Table *table);
ExecuteResult execute_select(Statement *statement, Table *table);
ExecuteResult execute_statement(Statement *statement, Table *table);
InputBuffer *new_input_buffer();
void read_input(InputBuffer *input_buffer);
void close_input_buffer(InputBuffer *input_buffer);
