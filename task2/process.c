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
#include <cursor.c>

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
        Cursor* cursor = table_end(table);

        Row *row_to_insert = &(statement->row_to_insert);
        serialize_row(row_to_insert, cursor_value(cursor));
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