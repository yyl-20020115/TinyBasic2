/*
 * Tiny BASIC Interpreter and Compiler Project
 * Interpreter module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 23-Aug-2019
 */


 /* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "interpret.h"
#include "errors.h"
#include "options.h"
#include "statement.h"


/* forward declarations */
static intptr_t interpret_expression(ExpressionNode* expression);
static void interpret_statement(StatementNode* statement);


/*
 * Data Definitions
 */

 /* The GOSUB Stack */
typedef struct gosub_stack_node GosubStackNode;
typedef struct gosub_stack_node {
	ProgramLineNode* program_line; /* the line following the GOSUB */
	GosubStackNode* next; /* stack node for the previous GOSUB */
} GosubStackNode;

/* private data */
typedef struct interpreter_data {
	ProgramNode* program; /* the program to interpret */
	ProgramLineNode* line; /* current line we're executing */
	GosubStackNode* gosub_stack; /* the top of the GOSUB stack */
	intptr_t gosub_stack_size; /* number of entries on the GOSUB stack */
	intptr_t variables[26]; /* the numeric variables */
	intptr_t stopped; /* set to 1 when an END is encountered */
	ErrorHandler* errors; /* the error handler */
	LanguageOptions* options; /* the language options */
} InterpreterData;

/* convenience variables */
static Interpreter* this; /* the object we are working with */


/*
 * Private Methods
 */


 /*
  * Evaluate a factor for the interpreter
  * params:
  *   FactorNode*   factor   the factor to evaluate
  */
static intptr_t interpret_factor(FactorNode* factor) {

	/* local variables */
	intptr_t result_store = 0; /* result of factor evaluation */

	/* check factor class */
	switch (factor->class) {

		/* a regular variable */
	case FACTOR_VARIABLE:
		result_store = this->priv->variables[factor->data.variable - 1]
			* (factor->sign == SIGN_POSITIVE ? 1 : -1);
		break;

		/* an integer constant */
	case FACTOR_VALUE:
		result_store = factor->data.value
			* (factor->sign == SIGN_POSITIVE ? 1 : -1);
		break;

		/* an expression */
	case FACTOR_EXPRESSION:
		result_store = interpret_expression(factor->data.expression)
			* (factor->sign == SIGN_POSITIVE ? 1 : -1);
		break;

		/* this only happens if the parser has failed in its duty */
	default:
		this->priv->errors->set_code
		(this->priv->errors, E_INVALID_EXPRESSION, 0, 0, this->priv->line->label);
	}
#ifdef USE_LIMIT_RESULT
	/* check the result and return it*/
	if (result_store < -32768 || result_store > 32767)
		this->priv->errors->set_code
		(this->priv->errors, E_OVERFLOW, 0, 0, this->priv->line->label);
#endif
	return result_store;
}

/*
 * Evaluate a term for the interpreter
 * params:
 *   TermNode*   term   the term to evaluate
 */
static intptr_t interpret_term(TermNode* term) {

	/* local variables */
	intptr_t result_store; /* the partial evaluation */
	RightHandFactor* rhfactor; /* pointer to successive rh factor nodes */
	intptr_t divisor; /* used to check for division by 0 before attempting */

	/* calculate the first factor result */
	result_store = interpret_factor(term->factor);
	rhfactor = term->next;

	/* adjust store according to successive rh factors */
	while (rhfactor && !this->priv->errors->get_code(this->priv->errors)) {
		switch (rhfactor->op) {
		case TERM_OPERATOR_MULTIPLY:
			result_store *= interpret_factor(rhfactor->factor);
#ifdef USE_LIMIT_RESULT
			if (result_store < -32768 || result_store > 32767)
				this->priv->errors->set_code
				(this->priv->errors, E_OVERFLOW, 0, 0, this->priv->line->label);
#endif
			break;
		case TERM_OPERATOR_DIVIDE:
			if ((divisor = interpret_factor(rhfactor->factor)))
				result_store /= divisor;
			else
				this->priv->errors->set_code
				(this->priv->errors, E_DIVIDE_BY_ZERO, 0, 0, this->priv->line->label);
			break;
		default:
			break;
		}
		rhfactor = rhfactor->next;
	}

	/* return the result */
	return result_store;
}

/*
 * Evaluate an expression for the interpreter
 * params:
 *   ExpressionNode*   expression   the expression to evaluate
 */
static intptr_t interpret_expression(ExpressionNode* expression) {

	/* local variables */
	intptr_t result_store; /* the partial evaluation */
	RightHandTerm* rhterm; /* pointer to successive rh term nodes */

	/* calculate the first term result */
	result_store = interpret_term(expression->term);
	rhterm = expression->next;

	/* adjust store according to successive rh terms */
	while (rhterm && !this->priv->errors->get_code(this->priv->errors)) {
		switch (rhterm->op) {
		case EXPRESSION_OPERATOR_PLUS:
			result_store += interpret_term(rhterm->term);
#ifdef USE_LIMIT_RESULT
			if (result_store < -32768 || result_store > 32767)
				this->priv->errors->set_code
				(this->priv->errors, E_OVERFLOW, 0, 0, this->priv->line->label);
#endif
			break;
		case EXPRESSION_OPERATOR_MINUS:
			result_store -= interpret_term(rhterm->term);
#ifdef USE_LIMIT_RESULT
			if (result_store < -32768 || result_store > 32767)
				this->priv->errors->set_code
				(this->priv->errors, E_OVERFLOW, 0, 0, this->priv->line->label);
#endif
			break;
		default:
			break;
		}
		rhterm = rhterm->next;
	}

	/* return the result */
	return result_store;
}

/*
 * Find a program line given its label
 * returns:
 *   ProgramLineNode*   the program line found
 */
static ProgramLineNode* find_label(intptr_t jump_label) {

	/* local variables */
	ProgramLineNode
		* ptr, /* a line we're currently looking at */
		* found = NULL; /* the line if found */

	/* do the search */
	for (ptr = this->priv->program->first; ptr && !found; ptr = ptr->next)
		if (ptr->label == jump_label)
			found = ptr;
		else if (ptr->label >= jump_label
			&& this->priv->options->get_line_numbers(this->priv->options)
			!= LINE_NUMBERS_OPTIONAL)
			found = ptr;

	/* check for errors and return what was found */
	if (!found)
		this->priv->errors->set_code
		(this->priv->errors, E_INVALID_LINE_NUMBER, 0, 0, this->priv->line->label);
	return found;
}


/*
 * Level 1 Routines
 */


 /*
  * Initialise the variables
  */
static void initialise_variables(void) {
	int count; /* counter for this->priv->variables */
	for (count = 0; count < 26; ++count) {
		this->priv->variables[count] = 0;
	}
}

/*
 * Interpret a LET statement
 * params:
 *   LetStatementNode*   letn   the LET statement details
 */
void interpret_let_statement(LetStatementNode* letn) {
	this->priv->variables[letn->variable - 1]
		= interpret_expression(letn->expression);
	this->priv->line = this->priv->line->next;
}

/*
 * Interpret a PEEK statement
 * params:
 *   PeekStatementNode*   peekn   the PEEK statement details
 */
void interpret_peek_statement(PeekStatementNode* peekn) {
	//get the int value inside the address
	intptr_t address = interpret_expression(peekn->address);
	//save the value to the variable
	this->priv->variables[peekn->variable - 1]
		= *((int*)address);
	this->priv->line = this->priv->line->next;
}

/*
 * Interpret a POKE statement
 * params:
 *   PokeStatementNode*   poken   the POKE statement details
 */
void interpret_poke_statement(PokeStatementNode* poken) {
	//get the int value inside the address
	intptr_t address = interpret_expression(poken->address);
	intptr_t value = interpret_expression(poken->value);
	//save the value to the address
	*(intptr_t*)address = value;

	this->priv->line = this->priv->line->next;
}


/*
 * Interpret an IF statement
 * params:
 *   IfStatementNode*   ifn   the IF statement details
 */
void interpret_if_statement(IfStatementNode* ifn) {

	/* local variables */
	intptr_t
		left, /* result of the left-hand expression */
		right, /* result of the right-hand expression */
		comparison; /* result of the comparison between the two */

	/* get the expressions */
	left = interpret_expression(ifn->left);
	right = interpret_expression(ifn->right);

	/* make the comparison */
	switch (ifn->op) {
	case RELOP_EQUAL: comparison = (left == right); break;
	case RELOP_UNEQUAL: comparison = (left != right); break;
	case RELOP_LESSTHAN: comparison = (left < right); break;
	case RELOP_LESSOREQUAL: comparison = (left <= right); break;
	case RELOP_GREATERTHAN: comparison = (left > right); break;
	case RELOP_GREATEROREQUAL: comparison = (left >= right); break;
	default: comparison = 0; break;
	}

	/* perform the conditional statement */
	if (comparison && !this->priv->errors->get_code(this->priv->errors))
		interpret_statement(ifn->statement);
	else
		this->priv->line = this->priv->line->next;
}

/*
 * Interpret a GOTO statement
 * params:
 *   GotoStatementNode*   goton   the GOTO statement details
 */
void interpret_goto_statement(GotoStatementNode* goton) {
	intptr_t label; /* the line label to go to */
	label = interpret_expression(goton->label);
	if (!this->priv->errors->get_code(this->priv->errors))
		this->priv->line = find_label(label);
}

/*
 * Interpret a GOSUB statement
 * params:
 *   GosubStatementNode*   gosubn   the GOSUB statement details
 */
void interpret_gosub_statement(GosubStatementNode* gosubn) {

	/* local variables */
	GosubStackNode* gosub_node; /* indicates the program line to return to */
	intptr_t label; /* the line label to go to */

	/* create the new node on the GOSUB stack */
	if (this->priv->gosub_stack_size < this->priv->options->get_gosub_limit
	(this->priv->options)) {
		gosub_node = malloc(sizeof(GosubStackNode));
		if (gosub_node != NULL) {
			gosub_node->program_line = this->priv->line->next;
			gosub_node->next = this->priv->gosub_stack;
			++this->priv->gosub_stack_size;
		}
		else {
			//unable to add stack
		}
		this->priv->gosub_stack = gosub_node;
	}
	else
		this->priv->errors->set_code(this->priv->errors,
			E_TOO_MANY_GOSUBS, 0, 0, this->priv->line->label);

	/* branch to the subroutine requested */
	if (!this->priv->errors->get_code(this->priv->errors))
		label = interpret_expression(gosubn->label);
	if (!this->priv->errors->get_code(this->priv->errors))
		this->priv->line = find_label(label);
}

/*
 * Interpret a RETURN statement
 */
void interpret_return_statement(void) {

	/* local variables */
	GosubStackNode* gosub_node; /* node popped off the GOSUB stack */

	/* return to the statement following the most recent GOSUB */
	if (this->priv->gosub_stack) {
		this->priv->line = this->priv->gosub_stack->program_line;
		gosub_node = this->priv->gosub_stack;
		this->priv->gosub_stack = this->priv->gosub_stack->next;
		free(gosub_node);
		--this->priv->gosub_stack_size;
	}

	/* no GOSUBs led here, so raise an error */
	else
		this->priv->errors->set_code
		(this->priv->errors, E_RETURN_WITHOUT_GOSUB, 0, 0, this->priv->line->label);
}

/*
 * Interpret a PRINT statement
 * params:
 *   PrintStatementNode*   printn   the PRINT statement details
 */
void interpret_print_statement(PrintStatementNode* printn) {

	/* local variables */
	OutputNode* outn; /* current output node */
	intptr_t
		items = 0, /* counter ensures runtime errors appear on a new line */
		result; /* the result of an expression */

	/* print each of the output items */
	outn = printn->first;
	while (outn) {
		switch (outn->class) {
		case OUTPUT_STRING:
			printf(_T("%s"), outn->output.string);
			++items;
			break;
		case OUTPUT_EXPRESSION:
			result = interpret_expression(outn->output.expression);
			if (!this->priv->errors->get_code(this->priv->errors)) {
				if (sizeof(intptr_t) == sizeof(int) << 1)
					printf(_T("%lld"), result);
				else
					printf(_T("%d"), (int)result);
				++items;
			}
			break;
		}
		outn = outn->next;
	}

	/* print the linefeed */
	if (items)
		printf(_T("\n"));
	this->priv->line = this->priv->line->next;
}

/*
 * Interpret an INPUT statement
 * params:
 *   InputStatementNode*   inputn   the INPUT statement details
 */
void interpret_input_statement(InputStatementNode* inputn) {

	/* local variables */
	VariableListNode* variable; /* current variable to input */
	intptr_t
		value, /* value input from the user */
		sign = 1, /* the default sign */
		ch = 0; /* character from the input stream */

	/* input each of the variables */
	variable = inputn->first;
	while (variable) {
		do {
			if (ch == _T('-')) sign = -1; else sign = 1;
			ch = getchar();
		} while (ch < _T('0') || ch > _T('9'));
		value = 0;
		do {
			value = 10 * value + (ch - _T('0'));
#ifdef USE_LIMIT_RESULT
			if (value * sign < -32768 || value * sign > 32767)
				this->priv->errors->set_code
				(this->priv->errors, E_OVERFLOW, 0, 0, this->priv->line->label);
#endif
			//NOTICE: use getchar to communicate with console
			ch = getchar();
		} while (ch >= _T('0') && ch <= _T('9')
			&& !this->priv->errors->get_code(this->priv->errors));
		this->priv->variables[variable->variable - 1] = sign * value;
		variable = variable->next;
	}

	/* advance to the next statement when done */
	this->priv->line = this->priv->line->next;
}


/*
 * Interpret an individual statement
 * params:
 *   StatementNode*   statement   the statement to interpret
 */
void interpret_statement(StatementNode* statement) {

	/* skip comments */
	if (!statement) {
		this->priv->line = this->priv->line->next;
		return;
	}

	/* interpret real statements */
	switch (statement->class) {
	case STATEMENT_NONE:
		break;
	case STATEMENT_LET:
		interpret_let_statement(statement->statement.letn);
		break;
	case STATEMENT_IF:
		interpret_if_statement(statement->statement.ifn);
		break;
	case STATEMENT_GOTO:
		interpret_goto_statement(statement->statement.goton);
		break;
	case STATEMENT_GOSUB:
		interpret_gosub_statement(statement->statement.gosubn);
		break;
	case STATEMENT_RETURN:
		interpret_return_statement();
		break;
	case STATEMENT_END:
		this->priv->stopped = 1;
		break;
	case STATEMENT_PRINT:
		interpret_print_statement(statement->statement.printn);
		break;
	case STATEMENT_INPUT:
		interpret_input_statement(statement->statement.inputn);
		break;
	case STATEMENT_POKE:
		interpret_poke_statement(statement->statement.poken);
		break;
	case STATEMENT_PEEK:
		interpret_peek_statement(statement->statement.peekn);
		break;
	default:
		printf(INTERPRETER_TYPE_ERROR, statement->class);
	}
}

/*
 * Interpret program starting from a particular line
 * params:
 *   ProgramLineNode*   program_line   the starting line
 */
static void interpret_program_from(ProgramLineNode* program_line) {
	this->priv->line = program_line;
	while (this->priv->line
		&& !this->priv->stopped
		&& !this->priv->errors->get_code(this->priv->errors))
		interpret_statement(this->priv->line->statement);
}


/*
 * Public Methods
 */


 /*
  * Interpret the program from the beginning
  * params:
  *   Interpreter*   interpreter   the interpreter to use
  *   ProgramNode*   program       the program to interpret
  */
static void interpret(Interpreter* interpreter, ProgramNode* program) {
	this = interpreter;
	this->priv->program = program;
	initialise_variables();
	interpret_program_from(this->priv->program->first);
}

/*
 * Destroy the interpreter
 * params:
 *   Interpreter*   interpreter   the doomed interpreter
 */
static void destroy(Interpreter* interpreter) {
	if (interpreter) {
		if (interpreter->priv)
			free(interpreter->priv);
		free(interpreter);
	}
}


/*
 * Constructors
 */


 /*
  * Constructor
  * returns:
  *   Interpreter*   the new interpreter
  */
Interpreter* new_Interpreter(ErrorHandler* errors, LanguageOptions* options) {

	/* allocate memory */
	this = malloc(sizeof(Interpreter));
	if (this == NULL) return NULL;
	this->priv = malloc(sizeof(InterpreterData));
	if (this->priv == NULL) {
		free(this);
		return NULL;
	}
	/* initialise methods */
	this->interpret = interpret;
	this->destroy = destroy;

	/* initialise properties */
	this->priv->gosub_stack = NULL;
	this->priv->gosub_stack_size = 0;
	this->priv->stopped = 0;
	this->priv->errors = errors;
	this->priv->options = options;

	/* return the new object */
	return this;
}
