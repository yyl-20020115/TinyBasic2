/*
 * Tiny BASIC
 * Tokenisation Header
 *
 * Copyright (C) Damian Walker 2019
 * Created: 04-Aug-2019
 */
#ifndef __TOKENISER_H__
#define __TOKENISER_H__
 /* pre-requisite headers */
#include <stdio.h>
#include "token.h"


/*
 * Structure Defnitions
 */


 /*
  * Data definitions
  */


  /* modes of reading */
typedef enum {
	DEFAULT_MODE, /* we have no idea what's coming */
	COMMENT_MODE, /* reading a comment */
	WORD_MODE, /* reading an identifier or keyword */
	NUMBER_MODE, /* reading a numeric constant */
	LESS_THAN_MODE, /* reading an operator staring with < */
	GREATER_THAN_MODE, /* reading an operator starting with > */
	STRING_LITERAL_MODE, /* reading a string literal */
	UNKNOWN_MODE /* we are lost */
} Mode;

/* current state information */
typedef struct {
	Token* token; /* token to return */
	Mode mode; /* current reading mode */
	int ch; /* last-read character */
	TCHAR tch;
	TCHAR* content; /* content of token under construction */
	int max; /* memory reserved for content */
} TokeniserState;

/* Token stream */
typedef struct token_stream TokenStream;
typedef struct token_stream {
  void *data; /* private data */
  Token *(*next) (TokenStream *);
  int (*get_line) (TokenStream *);
  void (*destroy) (TokenStream *);
} TokenStream;


/*
 * Constructor Declarations
 */


/*
 * Constructor for TokenStream
 * params:
 *   FILE*   input   Input file
 * returns:
 *   TokenStream*    The new token stream
 */
TokenStream *new_TokenStream (FILE *input);
TokenStream* new_BufferTokenStream(const TCHAR* input, int buffer_size);
TokenClass identify_symbol(int ch);
TokenClass identify_word(const TCHAR* word);
TokenClass identify_compound_symbol(const TCHAR* symbol);
#endif
