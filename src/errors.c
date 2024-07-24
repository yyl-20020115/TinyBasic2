/*
 * Tiny BASIC
 * Error Handling Module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 18-Aug-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errors.h"


/*
 * Internal Data Structures
 */


/* Private data */
typedef struct {
  ErrorCode error; /* the last error encountered */
  int line; /* the source line on which the error occurred */
  int column;
  int label; /* the label for the source line */
} FileTokenizerPrivateData;


/*
 * Internal Data
 */


/* convenience variables */
ErrorHandler *this; /* object being worked on */
FileTokenizerPrivateData *data; /* private data of object being worked on */

/* global variables */
static TCHAR *messages[E_LAST] = { /* the error messages */
ERROR_SUCCESSFUL,
ERROR_INVALID_LINE_NUMBER,
ERROR_UNRECOGNISED_COMMAND,
ERROR_INVALID_VARIABLE,
ERROR_INVALID_ASSIGNMENT,
ERROR_INVALID_EXPRESSION,
ERROR_MISSING,
ERROR_INVALID_PRINT_OUTPUT,
ERROR_BAD_COMMAND_LINE,
ERROR_FILE_NOT_FOUND,
ERROR_INVALID_OPERATOR,
ERROR_THEN_EXPECTED,
ERROR_UNEXPECTED_PARAMETER,
ERROR_RETURN_WITHOUT_GOSUB,
ERROR_DIVIDE_BY_ZERO,
ERROR_OVERFLOW
};


/*
 * Public Methods
 */


/*
 * Record an error encountered
 * globals:
 *   ErrorCode   error       the last error encountered
 *   int         line        the source line
 *   int         label       the line's label
 * params:
 *   ErrorCode   new_error   the error code to set
 *   int         new_line    the source line to set
 *   int         new_label   the label to set
 */
static void set_code (ErrorHandler *errors, ErrorCode new_error, int new_line, int new_column,
  int new_label) {

  /* initialise */
  this = errors;
  data = this->data;

  /* set the properties */
  data->error = new_error;
  data->line = new_line;
  data->column = new_column;
  data->label = new_label;
}

/*
 * Return the last error code encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   ErrorCode                the last error encountered
 */
static ErrorCode get_code (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->error;
}

/*
 * Return the last error line encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   int                      the source line of the last error
 */
static int get_line (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->line;
}

/*
 * Return the last error label encountered
 * params:
 *   ErrorHandler*   errors   the error handler
 * returns:
 *   int                      the line label of the last error
 */
static int get_label (ErrorHandler *errors) {
  this = errors;
  data = this->data;
  return data->label;
}

/*
 * Generate an error message
 * params:
 *   ErrorHandler*   errors     the error handler
 * globals:
 *   TCHAR*           messages   a list of error messages
 * returns:
 *   TCHAR*                      the full error message
 */
static TCHAR *get_text (ErrorHandler *errors) {

  /* local variables */
  TCHAR
    *message, /* the complete message */
    *line_text, /* source line N */
    *label_text; /* label N */

  /* initialise the error object */
  this = errors;
  data = this->data;

  /* get the source line, if there is one */
  line_text = _malloc (REPL_LINE_LENGTH);
  if (data->line&& line_text!=NULL)
    snprintf (line_text, 40, PROMPT_SOURCE_LINE, data->line, data->column);
  else if(line_text != NULL)
    strcpy (line_text, _T(""));

  /* get the source label, if there is one */
  label_text = _malloc (19);
  if (data->label && label_text != NULL)
    snprintf (label_text, 19, PROMPT_LINE_LABEL, data->label);
  else if(label_text != NULL)
    strcpy (label_text, _T(""));

  /* put the error message together */
  message = _malloc (strlen (messages[data->error]) + (line_text==NULL?0:strlen (line_text))
    + (label_text==NULL?0:strlen (label_text) + 1));
  if (message != NULL && line_text!=NULL && label_text!=NULL) {
      strcpy(message, messages[data->error]);
      strcat(message, line_text);
      strcat(message, label_text);
  }
  free (line_text);
  free (label_text);

  /* return the assembled error message */
  return message;
}

/*
 * ErrorHandler destructor
 * params:
 *   ErrorHandler*   errors   the doomed error handler
 */
static void destroy (ErrorHandler *errors) {
  if ((this = errors)) {
    data = this->data;
    free (data);
    free (this);
  }
}


/*
 * Constructors
 */


/*
 * Principal constructor
 * returns:
 *   ErrorHandler*   the new error handler object
 */
ErrorHandler *new_ErrorHandler (void) {

    /* allocate memory */
    this = malloc (sizeof (ErrorHandler));
    if (this == NULL) return NULL;
    this->data = data = malloc (sizeof (FileTokenizerPrivateData));
    if (data == NULL) return NULL;

    /* initialise the methods */
    this->set_code = set_code;
    this->get_code = get_code;
    this->get_line = get_line;
    this->get_label = get_label;
    this->get_text = get_text;
    this->destroy = destroy;

    /* initialise the properties */
    data->error = E_NONE;
    data->line = 0;
    data->label = 0;

    /* return the new object */
    return this;
}