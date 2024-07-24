/*
 * Tiny BASIC
 * Listing Output Module
 *
 * Released as Public Domain by Damian Gareth Walker, 2019
 * Created: 18-Sep-2019
 */


/* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "common.h"
#include "formatter.h"
#include "expression.h"
#include "errors.h"
#include "parser.h"


/*
 * Data Definitions
 */


/* private formatter data */
typedef struct formatter_data {
  ErrorHandler *errors; /* the error handler */
} FormatterData;

/* convenience variables */
static Formatter *this; /* the object being worked on */

/*
 * Forward References
 */


/* factor_output() has a forward reference to output_expression() */
static TCHAR *output_expression (ExpressionNode *expression);

/* output_statement() has a forward reference from output_if() */
static TCHAR *output_statement (StatementNode *statement);


/*
 * Functions
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
      factor_text = _malloc(2);
      if(factor_text!=NULL)
          snprintf (factor_text,2, _T("%c"), factor->data.variable + _T('A') - 1);
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
      this->priv->errors->set_code
	(this->priv->errors, E_INVALID_EXPRESSION, 0,0,0);
  }

  /* apply a negative sign, if necessary */
  if (factor_text && factor->sign == SIGN_NEGATIVE) {
    factor_buffer = _malloc (strlen (factor_text) + 2);
    if (factor_buffer != NULL)
        snprintf (factor_buffer, strlen(factor_text) + 2, _T("-%s"), factor_text);
    free (factor_text);
    factor_text = factor_buffer;
  }

  /* return the final factor representation */
  return factor_text;
}

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
    while (! this->priv->errors->get_code (this->priv->errors) && rhfactor) {

      /* ascertain the operator text */
      switch (rhfactor->op) {
      case TERM_OPERATOR_MULTIPLY:
        operator_char = '*';
        break;
      case TERM_OPERATOR_DIVIDE:
        operator_char = '/';
        break;
      default:
        this->priv->errors->set_code
	  (this->priv->errors, E_INVALID_EXPRESSION, 0, 0, 0);
        free (term_text);
        term_text = NULL;
      }

      /* get the factor that follows the operator */
      if (! this->priv->errors->get_code (this->priv->errors)
        && (factor_text = output_factor (rhfactor->factor))) {
        term_text = term_text==NULL?NULL:((TCHAR*)_realloc (term_text,
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
    operator_char; /* the operator that joins the righthand term */
  RightHandTerm *rhterm; /* right hand terms of the expression */

  /* begin with the initial term */
  if ((expression_text = output_term (expression->term))) {
    rhterm = expression->next;
    while (! this->priv->errors->get_code (this->priv->errors) && rhterm) {

      /* ascertain the operator text */
      switch (rhterm->op) {
      case EXPRESSION_OPERATOR_PLUS:
        operator_char = '+';
        break;
      case EXPRESSION_OPERATOR_MINUS:
        operator_char = '-';
        break;
      default:
        this->priv->errors->set_code
	  (this->priv->errors, E_INVALID_EXPRESSION, 0, 0, 0);
        free (expression_text);
        expression_text = NULL;
      }

      /* get the terms that follow the operators */
      if (! this->priv->errors->get_code (this->priv->errors)
        && (term_text = output_term (rhterm->term))) {
        expression_text = expression_text== NULL ? NULL : ((TCHAR*)_realloc (expression_text,
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
    let_text = _malloc (7 + strlen (expression_text));
    if(let_text!=NULL)
        snprintf (let_text, 7 + strlen(expression_text), FORMATTER_LET, _T('A') - 1 + letn->variable, expression_text);
    free (expression_text);
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
  if (op_text == NULL) return NULL;
  switch (ifn->op) {
    case RELOP_EQUAL: strcpy (op_text, _T("=")); break;
    case RELOP_UNEQUAL: strcpy (op_text, _T("<>")); break;
    case RELOP_LESSTHAN: strcpy (op_text, _T("<")); break;
    case RELOP_LESSOREQUAL: strcpy (op_text, _T("<=")); break;
    case RELOP_GREATERTHAN: strcpy (op_text, _T(">")); break;
    case RELOP_GREATEROREQUAL: strcpy (op_text, _T(">=")); break;
  }

  /* assemble the final IF text, if we have everything we need */
  if (left_text && op_text && right_text && statement_text) {
    if_text = _malloc (3 + strlen (left_text) + strlen (op_text) +
      strlen (right_text) + 6 + strlen (statement_text) + 1);
    if(if_text!=NULL)
        snprintf (if_text, 3 + strlen(left_text) + strlen(op_text) +
            strlen(right_text) + 6 + strlen(statement_text) + 1,
            FORMATTER_IF_THEN, left_text, op_text, right_text,
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
    goto_text = _malloc (6 + strlen (expression_text));
    if(goto_text!=NULL)
        snprintf (goto_text, 6 + strlen(expression_text),FORMATTER_GOTO, expression_text);
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
    gosub_text = _malloc (7 + strlen (expression_text));
    if(gosub_text !=NULL)
        snprintf (gosub_text, 7 + strlen(expression_text), FORMATTER_GOSUB, expression_text);
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
  end_text = _malloc (4);
  if(end_text!=NULL)
      strcpy (end_text, KEYWORD_END);
  return end_text;
}


/*
 * RETURN statement output
 * returns:
 *   TCHAR*   A new string with the text "RETURN"
 */
static TCHAR *output_return (void) {
  TCHAR *return_text; /* the full text of the RETURN command */
  return_text = _malloc (7);
  if(return_text!=NULL)
      strcpy (return_text, KEYWORD_RETURN);
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
    *print_text, /* the PRINT text to be assembled */
    *output_text = NULL; /* the text of the current output item */
  OutputNode *output; /* the current output item */

  /* initialise the PRINT statement */
  print_text = _malloc (6);
  if (print_text == NULL) return NULL;
  strcpy (print_text, KEYWORD_PRINT);

  /* add the output items */
  if ((output = printn->first)) {
    do {
      if (print_text == NULL) break;
      /* add the separator */
      print_text = print_text==NULL?NULL:((TCHAR*)_realloc (print_text, strlen (print_text) + 2));
      if (print_text == NULL) return NULL;
          strcat (print_text, output == printn->first ? _T(" ") : _T(","));

      /* format the output item */
      switch (output->class) {
      case OUTPUT_STRING:
        output_text = (TCHAR*)_malloc (strlen (output->output.string) + 3);
        if(output_text!=NULL)
            snprintf (output_text, strlen(output->output.string) + 3,_T("%c%s%c"), _T('"'), output->output.string, _T('"'));
        break;
      case OUTPUT_EXPRESSION:
        output_text = output_expression (output->output.expression);
        break;
      }
      if (output_text != NULL) {
          /* add the output item */
          print_text = (TCHAR*)_realloc(print_text,
              strlen(print_text) + strlen(output_text) + 1);
          if (print_text != NULL && output_text != NULL)
              strcat(print_text, output_text);
          free(output_text);
      }

    /* look for the next output item */
    } while ((output = output->next));
  }

  /* return the assembled text */
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
    *input_text, /* the INPUT text to be assembled */
    var_text[3]; /* text representation of each variable with separator */
  VariableListNode *variable; /* the current output item */

  /* initialise the INPUT statement */
  input_text = _malloc (6);
  if(input_text!=NULL)
      strcpy (input_text, KEYWORD_INPUT);

  /* add the output items */
  if ((variable = inputn->first)) {
    do {
      snprintf (var_text,3, _T("%c%c"),
        (variable == inputn->first) ? _T(' ') : _T(','),
        variable->variable + _T('A') - 1);
      if(input_text!=NULL)
        input_text = input_text==NULL?NULL:(_realloc (input_text, strlen (input_text) + 3));
      if (input_text != NULL)
          strcat (input_text, var_text);
    } while ((variable = variable->next));
  }

  /* return the assembled text */
  return input_text;
}

/*
 * PEEK statement output
 * params:
 *   PeekStatementNode*   peekn   data for the PEEK statement
 * returns:
 *   TCHAR*                      the PEEK statement text
 */
static TCHAR* output_peek(PeekStatementNode* peekn) {

    /* local variables */
    TCHAR
        * peek_text = NULL, /* the PEEK text to be assembled */
        * address_text = NULL; /* the text of the expression */

    /* assemble the expression */
    address_text = output_expression(peekn->address);

    /* assemble the final LET text, if we have an expression */
    if (address_text) {
        peek_text = _malloc(8 + strlen(address_text));
        if (peek_text != NULL)
            snprintf(peek_text, 8 + strlen(address_text), FORMATTER_PEEK, _T('A') - 1 + peekn->variable, address_text);
        free(address_text);
    }

    /* return it */
    return peek_text;
}
/*
 * PEEK statement output
 * params:
 *   PeekStatementNode*   peekn   data for the PEEK statement
 * returns:
 *   TCHAR*                      the PEEK statement text
 */
static TCHAR* output_poke(PokeStatementNode* poken) {

    /* local variables */
    TCHAR
        * poke_text = NULL, /* the PEEK text to be assembled */
        * address_text = NULL, /* the text of the expression */
        *value_text = NULL;
    /* assemble the expression */
    address_text = output_expression(poken->address);
    value_text = output_expression(poken->value);
    /* assemble the final POKE text, if we have an expression */
    if (address_text) {
        poke_text = _malloc(8 + strlen(address_text) + strlen(value_text));
        if (poke_text != NULL)
            snprintf(poke_text, 8 + strlen(address_text), FORMATTER_POKE, value_text, address_text);
        free(address_text);
    }

    /* return it */
    return poke_text;
}


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
    case STATEMENT_PEEK:
        output = output_peek(statement->statement.peekn);
        break;
    case STATEMENT_POKE:
        output = output_poke(statement->statement.poken);
        break;
    default:
      output = _malloc (24);
      if(output!=NULL)
      strcpy (output, FORMATTER_UNRECOGNISED);
  }

  /* return the listing line */
  return output;
}

/*
 * Program Line Output
 * params:
 *   ProgramLineNode*   program_line     the line to output
 */
static void generate_line (ProgramLineNode *program_line) {

  /* local variables */
  TCHAR
    label_text [7], /* line label text */
    *output = NULL, /* the rest of the output */
    *line_text = NULL; /* the assembled line */

  /* initialise the line label */
  if (program_line->label)
    snprintf (label_text, 7, _T("%5d "), program_line->label);
  else
    strcpy (label_text,  _T("      "));

  /* build the statement itself */
  output = output_statement (program_line->statement);

  /* if this wasn't a comment, add it to the program */
  if (output) {
    line_text = _malloc (strlen (label_text) + strlen (output) + 2);
    if(line_text!=NULL)
        snprintf (line_text, strlen(label_text) + strlen(output) + 2, _T("%s%s\n"), label_text, output);
    free (output);
    this->output = this->output==NULL?NULL:((TCHAR*)_realloc (this->output,
        strlen(this->output) + (line_text == NULL ? 0 : strlen(line_text)) + 1));
    if(this->output!=NULL && line_text!=NULL)
        strcat (this->output, line_text);
    free (line_text);
  }
}


/*
 * Public Methods
 */


/*
 * Create a formatted version of the program
 * params:
 *   Formatter*     fomatter   the formatter
 *   ProgramNode*   program    the syntax tree
 */
static void generate (Formatter *formatter, ProgramNode *program) {

  /* local variables */
  ProgramLineNode *program_line; /* line to process */

  /* initialise this object */
  this = formatter;

  /* generate the code for the lines */
  program_line = program->first;
  while (program_line) {
    generate_line (program_line);
    program_line = program_line->next;
  }
}

/*
 * Destroy the formatter when no longer needed
 * params:
 *   Formatter*   formatter   the doomed formatter
 */
static void destroy (Formatter *formatter) {
  if (formatter) {
    if (formatter->output)
      free (formatter->output);
    if (formatter->priv)
      free (formatter->priv);
    free (formatter);
  }
}


/*
 * Constructors
 */


/*
 * The Formatter constructor
 * params:
 *   ErrorHandler   *errors   the error handler object
 * returns:
 *   Formatter*               the new formatter
 */
Formatter *new_Formatter (ErrorHandler *errors) {

  /* allocate memory */
  this = malloc (sizeof (Formatter));
  if (this == NULL)return NULL;
  this->priv = malloc (sizeof (FormatterData));
  if (this->priv == NULL) {
      free(this);
      return NULL;
  }
  /* initialise methods */
  this->generate = generate;
  this->destroy = destroy;

  /* initialise properties */
  this->output = malloc (sizeof (TCHAR));
  if (this->output == NULL)
  {
      free(this->priv);
      free(this);
      return NULL;
  }
  *this->output = '\0';
  this->priv->errors = errors;

  /* return the new object */
  return this;
}
