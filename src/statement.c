/*
 * Tiny BASIC
 * Statement Handling Module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 15-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "statement.h"


/*
 * LET Statement Functions
 */


/*
 * LET statement constructor
 * returns:
 *   LetStatementNode*   the created LET statement
 */
LetStatementNode *statement_create_let (void) {

  /* local variables */
  LetStatementNode *letn; /* the created node */

  /* allocate memory and assign safe defaults */
  letn = malloc (sizeof (LetStatementNode));
  if (letn == NULL) return NULL;
  letn->variable = 0;
  letn->expression = NULL;

  /* return the LET statement node */
  return letn;
}

/*
 * Destructor for a LET statement
 * params:
 *   LetStatementNode   *letn   the doomed LET statement.
 */
void statement_destroy_let (LetStatementNode *letn) {
  if (letn->expression)
    expression_destroy (letn->expression);
  free (letn);
}


/*
 * POKE statement constructor
 * returns:
 *   PokeStatementNode*   the created POKE statement
 */
PokeStatementNode* statement_create_poke(void) {

    /* local variables */
    PokeStatementNode* poken; /* the created node */

    /* allocate memory and assign safe defaults */
    poken = malloc(sizeof(PokeStatementNode));
    if (poken == NULL) return NULL;
    poken->address = NULL;
    poken->value = NULL;

    /* return the POKE statement node */
    return poken;
}

/*
 * Destructor for a POKE statement
 * params:
 *   PokeStatementNode   *poken   the doomed POKE statement.
 */
void statement_destroy_poke(PokeStatementNode* poken) {
    if (poken->address)
        expression_destroy(poken->address);
    if (poken->value)
        expression_destroy(poken->value);
    free(poken);
}
/*
 * PEEK statement constructor
 * returns:
 *   PeekStatementNode*   the created PEEK statement
 */
PeekStatementNode* statement_create_peek(void) {

    /* local variables */
    PeekStatementNode* peekn; /* the created node */

    /* allocate memory and assign safe defaults */
    peekn = malloc(sizeof(PeekStatementNode));
    if (peekn == NULL) return NULL;
    peekn->address = NULL;
    peekn->variable = 0;

    /* return the POKE statement node */
    return peekn;
}

/*
 * Destructor for a PEEK statement
 * params:
 *   PokeStatementNode   *peekn   the doomed POKE statement.
 */
void statement_destroy_peek(PeekStatementNode* poken) {
    if (poken->address)
        expression_destroy(poken->address);
    free(poken);
}

/*
 * IF Statement Functions
 */


/*
 * IF statement constructor
 * returns:
 *   IfStatementNode*   the created IF statement
 */
IfStatementNode *statement_create_if (void) {

  /* local variables */
  IfStatementNode *ifn; /* the created node */

  /* allocate memory and assign safe defaults */
  ifn = malloc (sizeof (IfStatementNode));
  ifn->left = ifn->right = NULL;
  ifn->op = RELOP_EQUAL;
  ifn->statement = NULL;

  /* return the IF statement node */
  return ifn;
}

/*
 * IF statement destructor
 * params:
 *   IfStatementNode*   ifn   the doomed IF statement
 */
void statement_destroy_if (IfStatementNode *ifn) {
  if (ifn->left)
    expression_destroy (ifn->left);
  if (ifn->right)
    expression_destroy (ifn->right);
  if (ifn->statement)
    statement_destroy (ifn->statement);
  free (ifn);
}


/*
 * GOTO Statement Functions
 */


/*
 * GOTO Statement Constructor
 * returns:
 *   GotoStatementNode*   the new GOTO statement
 */
GotoStatementNode *statement_create_goto (void) {

  /* local variables */
  GotoStatementNode *goton; /* the statement to create */

  /* create and initialise the data */
  goton = malloc (sizeof (GotoStatementNode));
  if (goton == NULL) return NULL;
  goton->label = NULL;

  /* return the goto statement */
  return goton;
}

/*
 * GOTO Statement Destructor
 * params:
 *   GotoStatementNode*   goton   the doomed GOTO statement
 */
void statement_destroy_goto (GotoStatementNode *goton) {
  if (goton) {
    if (goton->label)
      expression_destroy (goton->label);
    free (goton);
  }
}


/*
 * GOSUB Statement Functions
 */


/*
 * GOSUB Statement Constructor
 * returns:
 *   GosubStatementNode*   the new GOSUB statement
 */
GosubStatementNode *statement_create_gosub (void) {

  /* local variables */
  GosubStatementNode *gosubn; /* the statement to create */

  /* create and initialise the data */
  gosubn = malloc (sizeof (GosubStatementNode));
  if (gosubn == NULL) return NULL;
  gosubn->label = NULL;

  /* return the gosub statement */
  return gosubn;
}

/*
 * GOSUB Statement Destructor
 * params:
 *   GosubStatementNode*   gosubn   the doomed GOSUB statement
 */
void statement_destroy_gosub (GosubStatementNode *gosubn) {
  if (gosubn) {
    if (gosubn->label)
      expression_destroy (gosubn->label);
    free (gosubn);
  }
}


/*
 * PRINT Statement Functions
 */


/*
 * PRINT statement constructor
 * returns:
 *   PrintStatementNode*   the created PRINT statement
 */
PrintStatementNode *statement_create_print (void) {

  /* local variables */
  PrintStatementNode *printn; /* the created node */

  /* allocate memory and assign safe defaults */
  printn = malloc (sizeof (PrintStatementNode));
  if (printn == NULL) return NULL;
  printn->first = NULL;

  /* return the PRINT statement node */
  return printn;
}

/*
 * Destructor for a PRINT statement
 * params:
 *   PrintStatementNode   *printn   the doomed PRINT statement.
 */
void statement_destroy_print (PrintStatementNode *printn) {
  OutputNode *current, *next;
  current = printn->first;
  while (current) {
    next = current->next;
    if (current->class == OUTPUT_STRING)
      free (current->output.string);
    else if (current->class == OUTPUT_EXPRESSION)
      expression_destroy (current->output.expression);
    free (current);
    current = next;
  }
  free (printn);
}


/*
 * INPUT Statement Functions
 */


/*
 * INPUT statement constructor
 * returns:
 *   InputStatementNode*   initialised INPUT statement data
 */
InputStatementNode *statement_create_input (void) {

  /* local variables */
  InputStatementNode *inputn; /* the new input statement data */

  /* allocate memory and initalise safely */
  inputn = malloc (sizeof (InputStatementNode));
  if (inputn == NULL) return NULL;
  inputn->first = NULL;

  /* return the created node */
  return inputn;
}

/*
 * INPUT statement destructor
 * params:
 *   InputStatementNode*   inputn   the doomed INPUT statement node
 */
void statement_destroy_input (InputStatementNode *inputn) {

  /* local variables */
  VariableListNode
    *variable, /* the current variable to destroy */
    *next; /* the next variable to destroy */

  /* delete the variables from the variable list, then the input node */
  if (inputn) {
    variable = inputn->first;
    while (variable) {
      next = variable->next;
      free (variable);
      variable = next;
    }
    free (inputn);
  }
}


/*
 * Top Level Functions
 */


/*
 * Statement constructor
 * returns:
 *   StatementNode*   the newly-created blank statement
 */
StatementNode *statement_create (void) {

  /* local variables */
  StatementNode *statement; /* the created statement */

  /* allocate memory and set defaults */
  statement = malloc (sizeof (StatementNode));
  if (statement == NULL)return NULL;
  statement->class = STATEMENT_NONE;

  /* return the created statement */
  return statement;
}

/*
 * Statement destructor
 * params:
 *   StatementNode*   statement   the doomed statement
 */
void statement_destroy (StatementNode *statement) {
  switch (statement->class) {
    case STATEMENT_LET:
      statement_destroy_let (statement->statement.letn);
      break;
    case STATEMENT_PRINT:
      statement_destroy_print (statement->statement.printn);
      break;
    case STATEMENT_INPUT:
      statement_destroy_input (statement->statement.inputn);
      break;
    case STATEMENT_IF:
      statement_destroy_if (statement->statement.ifn);
      break;
    case STATEMENT_GOTO:
      statement_destroy_goto (statement->statement.goton);
      break;
    case STATEMENT_GOSUB:
      statement_destroy_gosub (statement->statement.gosubn);
      break;
    case STATEMENT_PEEK:
        statement_destroy_peek(statement->statement.peekn);
        break;
    case STATEMENT_POKE:
        statement_destroy_poke(statement->statement.poken);
        break;
    default:
      break;
  }
  free (statement);
}


/*
 * Program Line Constructor
 * returns:
 *   ProgramLineNode*   the new program line
 */
ProgramLineNode *program_line_create (void) {

  /* local variables */
  ProgramLineNode *program_line; /* the program line to create */

  /* create and initialise the program line */
  program_line = malloc (sizeof (ProgramLineNode));
  if (program_line == NULL)return NULL;
  program_line->label = 0;
  program_line->statement = NULL;
  program_line->next = NULL;

  /* return the new program line */
  return program_line;
}

/*
 * Program Line Destructor
 * params:
 *   ProgramLineNode*   program_line   the doomed program line
 * params:
 *   ProgramLineNode*                  the next program line
 */
ProgramLineNode *program_line_destroy (ProgramLineNode *program_line) {

  /* local variables */
  ProgramLineNode *next = NULL; /* the next program line */

  /* record the next line and destroy this one */
  if (program_line) {
    next = program_line->next;
    if (program_line->statement)
      statement_destroy (program_line->statement);
    free (program_line);
  }

  /* return the line following */
  return next;
}

/*
 * Program Constructor
 * returns:
 *   ProgramNode*   the constructed program
 */
ProgramNode *program_create (void) {

  /* local variables */
  ProgramNode *program; /* new program */

  /* create and initialise the program */
  program = malloc (sizeof (program));
  if (program == NULL)return NULL;
  program->first = NULL;

  /* return the new program */
  return program;
}

/*
 * Program Destructor
 * params:
 *   ProgramNode*   program   the doomed program
 */
void program_destroy (ProgramNode *program) {

  /* local variables */
  ProgramLineNode *program_line; /* the program line to destroy */

  /* destroy the program lines, then the program itself */
  program_line = program->first;
  while (program_line)
    program_line = program_line_destroy (program_line);
  free (program);
}
