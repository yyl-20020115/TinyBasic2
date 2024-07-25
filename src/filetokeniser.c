/*
 * Tiny BASIC
 * Tokenisation module
 *
 * Copyright (C) Damian Gareth Walker 2019
 * Created: 04-Aug-2019
 */


 /* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "tokenizer.h"



/* Private data */
typedef struct {
	FILE* input; /* the input file */
	int line, /* current line in the input file */
		pos, /* current position on the input line */
		start_line, /* line on which a token started */
		start_pos; /* position on which a token started */
} FileTokenizerPrivateData;


/*
 * File level variables
 */


 /* convenience variables */
static TokenStream* this; /* token stream passed in to public method */
static FileTokenizerPrivateData* data; /* private data for this */


/*
 * Level 2 Tokeniser Routines
 */


 /*
  * Read a character and update the position counter
  * globals:
  *   int               line    current line after character read
  *   int               pos     current character position after character read
  * params:
  *   TokeniserState*   state   current state of the tokeniser
  * returns:
  *   int              character just read
  */
static int read_character(TokeniserState* state, TCHAR* pch) {

	TCHAR cc = 0;
	int ch; /* character read from stream */

	/* read the character */
	ch = (cc = fgetc(data->input));
	if (pch != NULL) *pch = cc;
	/* update the position and line counters */
	if (ch == _T('\n')) {
		++data->line;
		data->pos = 0;
	}
	else {
		++data->pos;
	}

	/* return the character */
	return ch;
}

/*
 * Push a character back into the input stream and update position markers
 * globals:
 *   int               line    line number rolled back
 *   int               pos     character position rolled back
 * params:
 *   TokeniserState*   state   current state of the tokeniser
 */
static void unread_character(TokeniserState* state) {
	ungetc(state->ch, data->input);
	if (state->ch == _T('\n'))
		--data->line;
	else
		--data->pos;
}

/*
 * Append the last read character to the token content
 * params:
 *   TokeniserState*   state   current state of the tokeniser
 */
static void store_character(TokeniserState* state) {

	/* variable declarations */
	TCHAR* temp; /* temporary pointer to content */
	int length; /* current length of token */
	/* allocate more memory for the token content if necessary */
	if (strlen(state->content) == state->max - 1) {
		temp = state->content;
		state->content = _malloc(state->max);
		if (state->content != NULL) {
			strncpy(state->content, temp, state->max - 1);
			state->content[state->max - 1] = 0;
			state->max *= 2;
			free(temp);
		}
		else {
			state->content = temp;
		}
	}
	if (state != NULL && state->content != NULL) {
		/* now add the character to the token */
		length = (int)strlen(state->content);
		state->content[length++] = state->ch;
		state->content[length] = '\0';
	}
}

/*
 * Level 1 Tokeniser Routines
 */


 /*
  * Default mode - deal with character when state is unknown
  * globals:
  *   int               line         current line in the source file
  *   int               pos          current character position in the source
  *   int               start_line   line on which the current token started
  *   int               start_pos    char pos on which the current token started
  * params:
  *   TokeniserState*   state        current state of the tokeniser
  */
static void default_mode(TokeniserState* state) {

	/* deal with non-EOL whitespace */
	if (state->ch == _T(' ') ||
		state->ch == _T('\t')) {
		state->ch = read_character(state, &state->tch);
		data->start_line = data->line;
		data->start_pos = data->pos;
	}

	/* deal with EOL whitespace */
	else if (state->ch == _T('\n')) {
		data->start_line = data->line - 1;
		data->start_pos = data->pos;
		state->token = new_Token_init
		(TOKEN_EOL, data->start_line, data->start_pos, state->content);
	}

	/* alphabetic characters start a word */
	else if ((state->ch >= _T('A') && state->ch <= _T('Z')) ||
		(state->ch >= _T('a') && state->ch <= _T('z'))
		|| state->ch > 0x7f
		) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		state->mode = WORD_MODE;
	}

	/* digits start a number */
	else if (state->ch >= _T('0') && state->ch <= _T('9'))
		state->mode = NUMBER_MODE;

	/* check for tokens starting with less-than (<, <=, <>) */
	else if (state->ch == _T('<')) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		store_character(state);
		state->ch = read_character(state, &state->tch);
		state->mode = LESS_THAN_MODE;
	}

	/* check for tokens starting with greater-than (>, >=) */
	else if (state->ch == _T('>')) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		store_character(state);
		state->ch = read_character(state, &state->tch);
		state->mode = GREATER_THAN_MODE;
	}

	/* deal with other symbol operators */
	else if (strchr(_T("+-*/=(),"), state->ch) != NULL) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		store_character(state);
		state->token = new_Token_init(identify_symbol(state->ch),
			data->start_line, data->start_pos, state->content);
	}

	/* double quotes start a string literal */
	else if (state->ch == _T('"')) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		state->ch = read_character(state, &state->tch);
		state->mode = STRING_LITERAL_MODE;
	}

	/* detect end of file */
	else if (state->ch == EOF) {
		data->start_line = data->line;
		data->start_pos = data->pos;
		state->token = new_Token_init
		(TOKEN_EOF, data->start_line, data->start_pos, state->content);
	}

	/* other characters are illegal */
	else {
		data->start_line = data->line;
		data->start_pos = data->pos;
		store_character(state);
		state->token = new_Token_init
		(TOKEN_ILLEGAL, data->start_line, data->start_pos, state->content);
	}
}

/*
 * Word mode - deal with character when building a word token
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void word_mode(TokeniserState* state) {

	/* local variables */
	TokenClass class; /* recognised class of keyword */

	/* add letters and digits to the token */
	if ((state->ch >= _T('A') && state->ch <= _T('Z')) ||
		(state->ch >= _T('a') && state->ch <= _T('z'))
		|| state->ch >= 0x80
		) {
		store_character(state);
		state->ch = read_character(state, &state->tch);
	}

	/* other characters are pushed back for the next token */
	else {
		if (state->ch != EOF)
			unread_character(state);
		class = identify_word(state->content);
		if (class == TOKEN_REM) {
			*state->content = _T('\0');
			state->mode = COMMENT_MODE;
		}
		else
			state->token = new_Token_init
			(class, data->start_line, data->start_pos, state->content);
	}
}

/*
 * Comment mode - skip till end of line after a REM
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void comment_mode(TokeniserState* state) {
	if (state->ch == _T('\n'))
		state->mode = DEFAULT_MODE;
	else
		state->ch = read_character(state, &state->tch);
}

/*
 * Number mode - building a number token (integer only)
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void number_mode(TokeniserState* state) {

	/* add digits to the token */
	if (state->ch >= _T('0') && state->ch <= _T('9')) {
		store_character(state);
		state->ch = read_character(state, &state->tch);
	}

	/* other characters are pushed back for the next token */
	else {
		if (state->ch != EOF)
			unread_character(state);
		state->token = new_Token_init
		(TOKEN_NUMBER, data->start_line, data->start_pos, state->content);
	}

}

/*
 * Less than mode - checking for <> and <= operators
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void less_than_mode(TokeniserState* state) {
	if (state->ch == _T('=') || state->ch == _T('>'))
		store_character(state);
	else
		unread_character(state);
	state->token = new_Token_init
	(identify_compound_symbol(state->content), data->start_line,
		data->start_pos, state->content);
}

/*
 * Greater than mode - checking for >= and >< operators
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void greater_than_mode(TokeniserState* state) {
	if (state->ch == _T('=') || state->ch == _T('<'))
		store_character(state);
	else
		ungetc(state->ch, data->input);
	state->token = new_Token_init
	(identify_compound_symbol(state->content), data->start_line,
		data->start_pos, state->content);
}

/*
 * String literal mode - reading a string
 * globals:
 *   int               start_line   line on which the current token started
 *   int               start_pos    char pos on which the current token started
 * params:
 *   TokeniserState*   state        current state of the tokeniser
 */
static void string_literal_mode(TokeniserState* state) {

	/* a quote terminates the string */
	if (state->ch == _T('"'))
		state->token = new_Token_init
		(TOKEN_STRING, data->start_line, data->start_pos, state->content);

	/* a backslash escapes the next character */
	else if (state->ch == _T('\\')) {
		state->ch = read_character(state, &state->tch);
		store_character(state);
		state->ch = read_character(state, &state->tch);
	}

	/* EOF generates an error */
	else if (state->ch == EOF)
		state->token = new_Token_init
		(TOKEN_ILLEGAL, data->start_line, data->start_pos, state->content);

	/* all other characters are part of the string */
	else {
		store_character(state);
		state->ch = read_character(state, &state->tch);
	}
}


/*
 * Top Level Tokeniser Routines
 */


 /*
  * Get the next token
  * params:
  *   TokenStream*   token_stream   the token stream being processed
  * returns:
  *   Token*                        the token built
  */
static Token* next(TokenStream* token_stream) {

	/* local variables */
	TokeniserState state; /* current state of reading */
	Token* return_token; /* token to return */

	/* initialise */
	this = token_stream;
	data = this->data;
	state.token = NULL;
	state.mode = DEFAULT_MODE;
	state.max = 1024;
	state.content = _malloc(state.max);
	if (state.content != NULL) {
		memset(state.content, 0, sizeof(TCHAR) * state.max);
		//*(state.content) = _T('\0');
	}
	state.ch = read_character(&state, &state.tch);

	/* main loop */
	while (state.token == NULL) {
		switch (state.mode) {
		case DEFAULT_MODE:
			default_mode(&state);
			break;
		case COMMENT_MODE:
			comment_mode(&state);
			break;
		case WORD_MODE:
			word_mode(&state);
			break;
		case NUMBER_MODE:
			number_mode(&state);
			break;
		case LESS_THAN_MODE:
			less_than_mode(&state);
			break;
		case GREATER_THAN_MODE:
			greater_than_mode(&state);
			break;
		case STRING_LITERAL_MODE:
			string_literal_mode(&state);
			break;
		default:
			state.token = new_Token_init
			(TOKEN_EOF, data->start_line, data->start_pos, state.content);
			state.ch = EOF; /* temporary hack */
		}
	}

	/* store token and release state memory */
	return_token = state.token;
	free(state.content);

	/* return result */
	return return_token;

}

/*
 * Getter for the current line number
 * paramss:
 *   TokenStream*   token_stream   the token stream being processed
 * returns:
 *   int                           the current line number returned
 */
static int get_line(TokenStream* token_stream) {
	this = token_stream;
	data = this->data;
	return data->line;
}


/*
 * Constructors
 */


 /*
  * Constructor for TokenStream
  * params:
  *   FILE*   input   Input file
  * returns:
  *   TokenStream*    The new token stream
  */
TokenStream* new_TokenStream(FILE* input) {

	/* allocate the memory */
	this = malloc(sizeof(TokenStream));
	if (this == NULL) return NULL;
	this->data = data = malloc(sizeof(FileTokenizerPrivateData));
	if (data == NULL) {
		free(this);
		return NULL;
	}
	/* initialise methods */
	this->next = next;
	this->get_line = get_line;
	this->destroy = destroy_ts;

	/* initialise data */
	data->input = input;
	data->line = data->start_line = 1;
	data->pos = data->start_pos = 0;

	/* return new token stream */
	return this;
}
