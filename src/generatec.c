/*
 * Tiny BASIC Interpreter and Compiler Project
 * C Output Module
 *
 * Copyright (C) Damian Gareth Walker 2019
 * Created: 03-Oct-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "statement.h"
#include "expression.h"
#include "errors.h"
#include "parser.h"
#include "options.h"
#include "generatec.h"


#define GENERATOR_INCLUDE_STDIO   _T("#include <stdio.h>\n")
#define GENERATOR_INCLUDE_STDLIB  _T("#include <stdlib.h>\n")
#define GENERATOR_DEFINE_ERROR    _T("#define E_RETURN_WITHOUT_GOSUB %d\n")


/*
 * Internal Data
 */


/* label list */
typedef struct label {
  int number; /* the label number */
  struct label *next; /* the next label */
} CLabel;

/* private data */
typedef struct {
  unsigned int input_used:1; /* true if we need the input routine */
  unsigned long int vars_used:26; /* true for each variable used */
  CLabel *first_label; /* the start of a list of labels */
  TCHAR *code; /* the main block of generated code */
  ErrorHandler *errors; /* error handler for compilation */
  LanguageOptions *options; /* the language options for compilation */
} FileTokenizerPrivateData;

/* convenience variables */
static CProgram *this; /* the object being worked on */
static FileTokenizerPrivateData *data; /* the private data of the object */
static ErrorHandler *errors; /* the error handler */
static LanguageOptions *options; /* the language options */


/*
 * Forward References
 */


/* factor_output() has a forward reference to output_expression() */
static TCHAR *output_expression (ExpressionNode *expression);

/* output_statement() has a forward reference from output_if() */
static TCHAR *output_statement (StatementNode *statement);


/*
 * Level 6 Functions
 */


/*
 * Output a factor
 * params:
 *   FactorNode*   factor   the factor to output
 * return:
 *   TCHAR*                  the text representation of the factor
 */
static TCHAR *output_factor (FactorNode *factor) {

  /* local variables */
  TCHAR *factor_text = NULL, /* the text of the whole factor */
    *factor_buffer = NULL, /* temporary buffer for prepending to factor_text */
    *expression_text = NULL; /* the text of a subexpression */

  /* work out the main factor text */
  switch (factor->class) {
    case FACTOR_VARIABLE:
      factor_text = _malloc (2);
      if(factor_text!=NULL)
          snprintf (factor_text, 2, _T("%c"), factor->data.variable + _T('a') - 1);
      data->vars_used |= 1 << (factor->data.variable - 1);
      break;
    case FACTOR_VALUE:
      factor_text = _malloc (7);
      if (factor_text != NULL)
          snprintf (factor_text, 7, _T("%d"), factor->data.value);
      break;
    case FACTOR_EXPRESSION:
      if ((expression_text = output_expression (factor->data.expression))) {
        factor_text = _malloc (strlen (expression_text) + 3);
        if (factor_text != NULL)
            snprintf (factor_text, strlen(expression_text) + 3, _T("(%s)"), expression_text);
        free (expression_text);
      }
      break;
    default:
      errors->set_code (errors, E_INVALID_EXPRESSION, 0, 0, 0);
  }

  /* apply a negative sign, if necessary */
  if (factor_text && factor->sign == SIGN_NEGATIVE) {
    factor_buffer = _malloc (strlen (factor_text) + 2);
    if(factor_buffer!=NULL)
        snprintf (factor_buffer, strlen(factor_text) + 2, _T("-%s"), factor_text);
    free (factor_text);
    factor_text = factor_buffer;
  }

  /* return the final factor representation */
  return factor_text;
}


/*
 * Level 5 Functions
 */


/*
 * Output a term
 * params:
 *   TermNode*   term   the term to output
 * returns:
 *   TCHAR*              the text representation of the term
 */
static TCHAR *output_term (TermNode *term) {

  /* local variables */
  TCHAR
    *term_text = NULL, /* the text of the whole term */
    *factor_text = NULL, /* the text of each factor */
    operator_char; /* the operator that joins the righthand factor */
  RightHandFactor *rhfactor; /* right hand factors of the expression */

  /* begin with the initial factor */
  if ((term_text = output_factor (term->factor))) {
    rhfactor = term->next;
    while (! errors->get_code (errors) && rhfactor) {

      /* ascertain the operator text */
      switch (rhfactor->op) {
      case TERM_OPERATOR_MULTIPLY:
        operator_char = '*';
        break;
      case TERM_OPERATOR_DIVIDE:
        operator_char = '/';
        break;
      default:
        errors->set_code (errors, E_INVALID_EXPRESSION, 0, 0, 0);
        free (term_text);
        term_text = NULL;
      }

      /* get the factor that follows the operator */
      if (! errors->get_code (errors)
        && (factor_text = output_factor (rhfactor->factor))) {
        term_text = term_text==NULL?NULL:(_realloc(term_text,
          strlen (term_text) + strlen (factor_text) + 2));
        if(term_text!=NULL)
            snprintf (term_text, strlen(term_text) + strlen(factor_text) + 2, _T("%s%c%s"), term_text, operator_char, factor_text);
        free (factor_text);
      }

      /* look for another term on the right of the expression */
      rhfactor = rhfactor->next;
    }
  }

  /* return the expression text */
  return term_text;
}


/*
 * Level 4 Functions
 */


/*
 * Output an expression for a program listing
 * params:
 *   ExpressionNode*   expression   the expression to output
 * returns:
 *   TCHAR*                          new string containint the expression text
 */
static TCHAR *output_expression (ExpressionNode *expression) {

  /* local variables */
  TCHAR
    *expression_text = NULL, /* the text of the whole expression */
    *term_text = NULL, /* the text of each term */
    operator_char=' '; /* the operator that joins the righthand term */
  RightHandTerm *rhterm; /* right hand terms of the expression */

  /* begin with the initial term */
  if ((expression_text = output_term (expression->term))) {
    rhterm = expression->next;
    while (! errors->get_code (errors) && rhterm) {

      /* ascertain the operator text */
      switch (rhterm->op) {
      case EXPRESSION_OPERATOR_PLUS:
        operator_char = '+';
        break;
      case EXPRESSION_OPERATOR_MINUS:
        operator_char = '-';
        break;
      default:
        errors->set_code (errors, E_INVALID_EXPRESSION, 0, 0, 0);
        free (expression_text);
        expression_text = NULL;
      }

      /* get the terms that follow the operators */
      if (! errors->get_code (errors)
        && (term_text = output_term (rhterm->term))) {
        expression_text = expression_text==NULL?NULL:(_realloc (expression_text,
          strlen (expression_text) + strlen (term_text) + 2));
        if(expression_text!=NULL)
        snprintf (expression_text, strlen(expression_text) + strlen(term_text) + 2, _T("%s%c%s"), expression_text, operator_char,
          term_text);
        free (term_text);
      }

      /* look for another term on the right of the expression */
      rhterm = rhterm->next;
    }
  }

  /* return the expression text */
  return expression_text;

}


/*
 * Level 3 Functions
 */


/*
 * LET statement output
 * params:
 *   LetStatementNode*   letn   data for the LET statement
 * returns:
 *   TCHAR*                      the LET statement text
 */
static TCHAR *output_let (LetStatementNode *letn) {

  /* local variables */
  TCHAR
    *let_text = NULL, /* the LET text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (letn->expression);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    let_text = _malloc (4 + strlen (expression_text));
    if(let_text!=NULL)
        snprintf (let_text, 4 + strlen(expression_text), _T("%c=%s;"),_T('a') - 1 + letn->variable, expression_text);
    free (expression_text);
    data->vars_used |= 1 << (letn->variable - 1);
  }

  /* return it */
  return let_text;
}

/*
 * IF statement output
 * params:
 *   IfStatementNode*   ifn   data for the IF statement
 * returns:
 *   TCHAR*                    the IF statement text
 */
static TCHAR *output_if (IfStatementNode *ifn) {

  /* local variables */
  TCHAR
    *if_text = NULL, /* the LET text to be assembled */
    *left_text = NULL, /* the text of the left expression */
    *op_text = NULL, /* the operator text */
    *right_text = NULL, /* the text of the right expression */
    *statement_text = NULL; /* the text of the conditional statement */

  /* assemble the expressions and conditional statement */
  left_text = output_expression (ifn->left);
  right_text = output_expression (ifn->right);
  statement_text = output_statement (ifn->statement);

  /* work out the operator text */
  op_text = _malloc (3);
  if (op_text != NULL) {
      switch (ifn->op) {
      case RELOP_EQUAL: strcpy(op_text, _T("==")); break;
      case RELOP_UNEQUAL: strcpy(op_text, _T("!=")); break;
      case RELOP_LESSTHAN: strcpy(op_text, _T("<")); break;
      case RELOP_LESSOREQUAL: strcpy(op_text, _T("<=")); break;
      case RELOP_GREATERTHAN: strcpy(op_text, _T(">")); break;
      case RELOP_GREATEROREQUAL: strcpy(op_text, _T(">=")); break;
      }
  }
  /* assemble the final IF text, if we have everything we need */
  if (left_text && op_text && right_text && statement_text) {
    if_text = _malloc (4 + strlen (left_text) + strlen (op_text) +
      strlen (right_text) + 3 + strlen (statement_text) + 2);
    if(if_text!=NULL)
    snprintf (if_text, 4 + strlen(left_text) + strlen(op_text) +
        strlen(right_text) + 3 + strlen(statement_text) + 2,_T("if (%s%s%s) {%s}"), left_text, op_text, right_text,
      statement_text);
  }

  /* free up the temporary bits of memory we've reserved */
  if (left_text) free (left_text);
  if (op_text) free (op_text);
  if (right_text) free (right_text);
  if (statement_text) free (statement_text);

  /* return it */
  return if_text;
}

/*
 * GOTO statement output
 * params:
 *   GotoStatementNode*   goton   data for the GOTO statement
 * returns:
 *   TCHAR*                        the GOTO statement text
 */
static TCHAR *output_goto (GotoStatementNode *goton) {

  /* local variables */
  TCHAR
    *goto_text = NULL, /* the GOTO text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (goton->label);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    goto_text = _malloc (27 + strlen (expression_text));
    if(goto_text!=NULL)
        snprintf (goto_text, 27 + strlen(expression_text), _T("label=%s; goto goto_block;"), expression_text);
    free (expression_text);
  }

  /* return it */
  return goto_text;
}

/*
 * GOSUB statement output
 * params:
 *   GosubStatementNode*   gosubn   data for the GOSUB statement
 * returns:
 *   TCHAR*                        the GOSUB statement text
 */
static TCHAR *output_gosub (GosubStatementNode *gosubn) {

  /* local variables */
  TCHAR
    *gosub_text = NULL, /* the GOSUB text to be assembled */
    *expression_text = NULL; /* the text of the expression */

  /* assemble the expression */
  expression_text = output_expression (gosubn->label);

  /* assemble the final LET text, if we have an expression */
  if (expression_text) {
    gosub_text = _malloc (12 + strlen (expression_text));
    if(gosub_text!=NULL)
        snprintf (gosub_text, 12 + strlen(expression_text), _T("bas_exec(%s);"), expression_text);
    free (expression_text);
  }

  /* return it */
  return gosub_text;
}

/*
 * END statement output
 * returns:
 *   TCHAR*   A new string with the text "END"
 */
static TCHAR *output_end (void) {
  TCHAR *end_text; /* the full text of the END command */
  end_text = _malloc (9);
  if(end_text!=NULL)
      strcpy (end_text, _T("exit(0);"));
  return end_text;
}

/*
 * RETURN statement output
 * returns:
 *   TCHAR*   A new string with the text "RETURN"
 */
static TCHAR *output_return (void) {
  TCHAR *return_text; /* the full text of the RETURN command */
  return_text = _malloc (8);
  if(return_text!=NULL)
      strcpy (return_text, _T("return;"));
  return return_text;
}

/*
 * PRINT statement output
 * params:
 *   PrintStatementNode*   printn   data for the PRINT statement
 * returns:
 *   TCHAR*                          the PRINT statement text
 */
static TCHAR *output_print (PrintStatementNode *printn) {

  /* local variables */
  TCHAR
    *format_text = NULL, /* the printf format string */
    *output_list = NULL, /* the printf output list */
    *output_text = NULL, /* a single output item */
    *print_text = NULL; /* the PRINT text to be assembled */
  OutputNode *output; /* the current output item */

  /* initialise format and output text */
  format_text = _malloc (1);
  *format_text = '\0';
  output_list = _malloc (1);
  *output_list = '\0';

  /* build the format and output text */
  if ((output = printn->first)) {
    do {

      /* format the output item */
      switch (output->class) {
        case OUTPUT_STRING:
          format_text = format_text==NULL?NULL:(_realloc (format_text,
            strlen (format_text) + strlen (output->output.string) + 1));
          if (format_text != NULL)
              strcat (format_text, output->output.string);
          break;
        case OUTPUT_EXPRESSION:
          format_text = format_text==NULL?NULL
              :(_realloc (format_text, strlen (format_text) + 3));
          if(format_text!=NULL)
              strcat (format_text, _T("%d"));
          output_text = output_expression (output->output.expression);
          output_list = output_list==NULL?NULL:(_realloc (output_list,
            strlen (output_list) + 1 + (output_text==NULL?0:strlen (output_text)) + 1));
          if (output_text != NULL && output_list!=NULL) {
              strcat(output_list, _T(","));
              strcat(output_list, output_text);
          }
          free (output_text);
          break;
      }

    /* look for the next output item */
    } while ((output = output->next));
  }

  /* assemble the whole print text and return it */
  print_text = _malloc (8 + (format_text==NULL?0:strlen (format_text)) + 3 
      + (output_list==NULL?0:strlen (output_list)) + 3);
  if(print_text!=NULL)
      snprintf (print_text, 8 + (format_text == NULL ? 0 : strlen(format_text)) + 3
          + (output_list == NULL ? 0 : strlen(output_list)) + 3,_T("printf(\"%s\\n\"%s);"), format_text, output_list);
  free (format_text);
  free (output_list);
  return print_text;
}

/*
 * INPUT statement output
 * params:
 *   InputStatementNode*   inputn   the input statement node to show
 * returns:
 *   TCHAR *                         the text of the INPUT statement
 */
static TCHAR *output_input (InputStatementNode *inputn) {

  /* local variables */
  TCHAR
    *var_text, /* input text for a single variable */
    *input_text; /* the INPUT text to be assembled */
  VariableListNode *variable; /* the current output item */

  /* generate an input line for each variable listed */
  input_text = _malloc (1);
  if(input_text!=NULL)
      *input_text = '\0';
  if ((variable = inputn->first)) {
    do {
      var_text = _malloc (18);
      if(var_text!=NULL)
      snprintf (var_text, 18,_T("%s%c = bas_input();"),
        (variable == inputn->first) ? _T("") : _T("\n"),
        variable->variable + _T('a') - 1);
      input_text = input_text==NULL?NULL:(_realloc (input_text,
        strlen (input_text) + (var_text==NULL?0: strlen (var_text)) + 1));
      if(input_text!=NULL&&var_text!=NULL)
          strcat (input_text, var_text);
      free (var_text);
      data->vars_used |= 1 << (variable->variable - 1);
    } while ((variable = variable->next));
  }
  data->input_used = 1;

  /* return the assembled text */
  return input_text;
}


/*
 * Level 2 Functions
 */


/*
 * Statement output
 * params:
 *   StatementNode*   statement   the statement to output
 * returns:
 *   TCHAR*                        a string containing the statement line
 */
static TCHAR *output_statement (StatementNode *statement) {

  /* local variables */
  TCHAR *output = NULL; /* the text output */

  /* return null output for comments */
  if (! statement)
    return NULL;

  /* build the statement itself */
  switch (statement->class) {
    case STATEMENT_LET:
      output = output_let (statement->statement.letn);
      break;
    case STATEMENT_IF:
      output = output_if (statement->statement.ifn);
      break;
    case STATEMENT_GOTO:
      output = output_goto (statement->statement.goton);
      break;
    case STATEMENT_GOSUB:
      output = output_gosub (statement->statement.gosubn);
      break;
    case STATEMENT_RETURN:
      output = output_return ();
      break;
    case STATEMENT_END:
     output = output_end ();
     break;
    case STATEMENT_PRINT:
      output = output_print (statement->statement.printn);
      break;
    case STATEMENT_INPUT:
      output = output_input (statement->statement.inputn);
      break;
    default:
      output = _malloc (24);
      if(output!=NULL)
          strcpy (output, _T("Unrecognised statement."));
  }

  /* return the listing line */
  return output;
}


/*
 * Level 1 Functions
 */


/*
 * Program Line Generation
 * params:
 *   ProgramLineNode*   program_line   the program line to convert
 */
static void generate_line (ProgramLineNode *program_line) {

  /* local variables */
  CLabel
    *prior_label, /* label before potential insertion point */
    *next_label, /* label after potential insertion point */
    *new_label; /* a label to insert */
  TCHAR
    label_text[12], /* text of a line label */
    *statement_text; /* the text of a statement */

  /* generate a line label */
  if (program_line->label) {

    /* insert the label into the label list */
    new_label = malloc (sizeof (CLabel));
    if (new_label != NULL) {
        new_label->number = program_line->label;
        new_label->next = NULL;
        prior_label = NULL;
        next_label = data->first_label;
        while (next_label && next_label->number < new_label->number) {
          prior_label = next_label;
          next_label = prior_label->next;
        }
        new_label->next = next_label;
        if (prior_label)
          prior_label->next = new_label;
        else
          data->first_label = new_label;
    }

    /* append the label to the code block */
    snprintf (label_text,12, _T("lbl_%d:\n"), program_line->label);
    data->code = data->code==NULL?NULL:(_realloc (data->code,
      strlen (data->code) + strlen (label_text) + 1));
    if(data->code!=NULL)
        strcat (data->code, label_text);
  }

  /* generate the statement, and append it if it is not a comment */
  statement_text = output_statement (program_line->statement);
  if (statement_text) {
    data->code = data->code==NULL?NULL:(_realloc (data->code,
      strlen (data->code) + strlen (statement_text) + 2));
    if (data->code != NULL) {
        strcat(data->code, statement_text);
        strcat(data->code, _T("\n"));
    }
    free (statement_text);
  }
}

/*
 * Generate the #include lines and #defines
 * changes:
 *   Private*   data   appends headers to the output
 */
static void generate_includes (void) {

  /* local variables */
  TCHAR
    include_text[1024], /* the whole include and #define text */
    define_text[80]; /* a single #define line */

  /* build up includes and defines */
  strcpy (include_text, GENERATOR_INCLUDE_STDIO);
  strcat (include_text, GENERATOR_INCLUDE_STDLIB);
  snprintf (define_text, 80, GENERATOR_DEFINE_ERROR, E_RETURN_WITHOUT_GOSUB);
  strcat (include_text, define_text);

  /* add the #includes and #defines to the output */
  this->c_output = this->c_output == NULL ? NULL
      :(_realloc(this->c_output, strlen(this->c_output)
    + strlen (include_text) + 1));
  if(this->c_output!=NULL)
      strcat (this->c_output, include_text);
}

/*
 * Generate the variable declarations
 * changes:
 *   Private*   data   appends declaration to the output
 */
static void generate_variables (void) {

  /* local variables */
  int vcount; /* variable counter */
  TCHAR
    var_text [12], /* individual variable text */
      declaration[60]; /* declaration text */

  /* build the declaration */
  *declaration = '\0';
  for (vcount = 0; vcount < 26; ++vcount) {
    if (data->vars_used & 1 << vcount) {
      if (*declaration)
        snprintf (var_text, 12,_T(",%c"), _T('a') + vcount);
      else
        snprintf (var_text, 12,_T("short int %c"), _T('a') + vcount);
      strcat (declaration, var_text);
    }
  }

  /* if there are any variables, add the declaration to the output */
  if (*declaration) {
    strcat (declaration, _T(";\n"));
    this->c_output = this->c_output==NULL?NULL:(_realloc(this->c_output, strlen (this->c_output)
      + strlen (declaration) + 1));
    strcat (this->c_output, declaration);
  }
}

/*
 * Generate the bas_input function
 * changes:
 *   Private*   data   appends declaration to the output
 */
static void generate_bas_input (void) {

  /* local variables */
  TCHAR function_text[1024]; /* the entire function */

  /* construct the function text */
  strcpy (function_text, _T("short int bas_input (void) {\n"));
  strcat (function_text, _T("short int ch, sign, value;\n"));
  strcat (function_text, _T("do {\n"));
  strcat (function_text, _T("if (ch == '-') sign = -1; else sign = 1;\n"));
  strcat (function_text, _T("ch = getchar ();\n"));
  strcat (function_text, _T("} while (ch < '0' || ch > '9');\n"));
  strcat (function_text, _T("value = 0;\n"));
  strcat (function_text, _T("do {\n"));
  strcat (function_text, _T("value = 10 * value + (ch - '0');\n"));
  strcat (function_text, _T("ch = getchar ();\n"));
  strcat (function_text, _T("} while (ch >= '0' && ch <= '9');\n"));
  strcat (function_text, _T("return sign * value;\n"));
  strcat (function_text, _T("}\n"));

  /* add the function text to the output */
  this->c_output = this->c_output==NULL?NULL:(_realloc(this->c_output, strlen (this->c_output)
    + strlen (function_text) + 1));
  if(this->c_output!=NULL)
      strcat (this->c_output, function_text);
}

/*
 * Generate the bas_exec function
 * changes:
 *   Private*   data   appends declaration to the output
 */
static void generate_bas_exec (void) {

  /* local variables */
  TCHAR
    *op, /* comparison operator to use for line numbers */
    goto_line[80], /* a line in the goto block */
    *goto_block, /* the goto block */
    *function_text; /* the complete function text */
  CLabel *label; /* label pointer for construction goto block */

  /* decide which operator to use for comparison */
  op = (options->get_line_numbers (options) == LINE_NUMBERS_OPTIONAL)
    ? _T("==")
    : _T("<=");

  /* create the goto block */
  goto_block = _malloc (128);
  strcpy (goto_block, _T("goto_block:\n"));
  strcat (goto_block, _T("if (!label) goto lbl_start;\n"));
  label = data->first_label;
  while (label) {
    snprintf (goto_line, 80,_T("if (label%s%d) goto lbl_%d;\n"),
      op, label->number, label->number);
    goto_block = goto_block==NULL?NULL:(_realloc (goto_block,
      strlen (goto_block) + strlen (goto_line) + 1));
    strcat (goto_block, goto_line);
    label = label->next;
  }
  goto_block = goto_block==NULL?NULL:(_realloc (goto_block, strlen (goto_block) + 12));
  if(goto_block!=NULL)
      strcat (goto_block, _T("lbl_start:\n"));

  /* put the function together */
  function_text = _malloc (28 + goto_block==NULL?0:strlen (goto_block) + strlen (data->code) + 3);
  if (function_text != NULL) {
      strcpy(function_text, _T("void bas_exec (int label) {\n"));
      strcat(function_text, goto_block);
      strcat(function_text, data->code);
      strcat(function_text, _T("}\n"));
  }
  /* add the function text to the output */
  this->c_output = this->c_output==NULL?NULL:(_realloc(this->c_output, this->c_output==NULL?0:strlen (this->c_output)
    + strlen (function_text) + 1));
  if(this->c_output!=NULL && function_text!=NULL)
      strcat (this->c_output, function_text);
  free (goto_block);
  free (function_text);
}

/*
 * Generate the main function
 * changes:
 *   Private*   data   appends declaration to the output
 */
void generate_main (void) {

  /* local variables */
  TCHAR function_text[1024]; /* the entire function */

  /* construct the function text */
  strcpy (function_text, _T("int main (void) {\n"));
  strcat (function_text, _T("bas_exec (0);\n"));
  strcat (function_text, _T("exit (E_RETURN_WITHOUT_GOSUB);\n"));
  strcat (function_text, _T("}\n"));

  /* add the function text to the output */
  this->c_output = this->c_output==NULL?NULL:(_realloc(this->c_output, strlen (this->c_output)
    + strlen (function_text) + 1));
  if(this->c_output!=NULL)
      strcat (this->c_output, function_text);
}


/*
 * Top Level Functions
 */


/*
 * Program Generation
 * params:
 *   ProgramNode*   program   the program parse tree to convert to C
 * returns:
 *   TCHAR*                    the program output
 */
static void generate (CProgram *c_program, ProgramNode *program) {

  /* local variables */
  ProgramLineNode *program_line; /* line to process */

  /* initialise this object */
  this = c_program;
  data = (FileTokenizerPrivateData *) c_program->private_data;

  /* generate the code for the lines */
  program_line = program->first;
  while (program_line) {
    generate_line (program_line);
    program_line = program_line->next;
  }

  /* put the code together */
  generate_includes ();
  generate_variables ();
  if (data->input_used)
    generate_bas_input ();
  generate_bas_exec ();
  generate_main ();
}

/*
 * Destructor
 * params:
 *   CProgram*   c_program   The C program to destroy
 */
static void destroy (CProgram *c_program) {

  /* local variables */
  CLabel
    *current_label, /* pointer to label to destroy */
    *next_label; /* pointer to next label to destroy */

  /* destroy the private data */
  this = c_program;
  if (this->private_data) {
    data = (FileTokenizerPrivateData *) c_program->private_data;
    next_label = data->first_label;
    while (next_label) {
      current_label = next_label;
      next_label = current_label->next;
      free (current_label);
    }
    if (data->code)
      free (data->code);
    free (data);
  }

  /* destroy the generated output */
  if (this->c_output)
    free (this->c_output);

  /* destroy the containing structure */
  free (c_program);
}

/*
 * Constructor
 * params:
 *   ErrorHandler*   compiler_errors   the error handler
 * changes:
 *   CProgram*       this              the object being created
 *   Private*        data              the object's private data
 * returns:
 *   CProgram*                         the created object
 */
CProgram *new_CProgram (ErrorHandler *compiler_errors,
  LanguageOptions *compiler_options) {

  /* allocate space */
  this = malloc (sizeof (CProgram));
  if (this == NULL) return NULL;
  this->private_data = data = malloc (sizeof (FileTokenizerPrivateData));
  if (data == NULL) {
      free(this);
      return NULL;
  }
  /* initialise methods */
  this->generate = generate;
  this->destroy = destroy;

  /* initialise properties */
  errors = data->errors = compiler_errors;
  options = data->options = compiler_options;
  data->input_used = 0;
  data->vars_used = 0;
  data->first_label = NULL;
  data->code = _malloc (1);
  if(data->code!=NULL)
      *data->code = '\0';
  this->c_output = _malloc (1);
  if(this->c_output!=NULL)
      *this->c_output = '\0';

  /* return the created structure */
  return this;
}
