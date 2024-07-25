#include <string.h>
#include "tokenizer.h"
#include "malloc.h"
/*
 * Identify the various recognised symbols
 * params:
 *   int   ch     the character to identify
 * returns:
 *   TokenClass   the token class recognised by the parser
 */
TokenClass identify_symbol(int ch) {
    switch (ch) {
    case _T('+'):
        return TOKEN_PLUS;
        break;
    case _T('-'):
        return TOKEN_MINUS;
        break;
    case _T('*'):
        return TOKEN_MULTIPLY;
        break;
    case _T('/'):
        return TOKEN_DIVIDE;
        break;
    case _T('='):
        return TOKEN_EQUAL;
        break;
    case _T('('):
        return TOKEN_LEFT_PARENTHESIS;
        break;
    case _T(')'):
        return TOKEN_RIGHT_PARENTHESIS;
        break;
    case _T(','):
        return TOKEN_COMMA;
        break;
    default:
        return TOKEN_SYMBOL;
    }
}

TokenClass identify_word(const TCHAR* word) {
    if (!tinybasic_strcmp(word, KEYWORD_LET) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_LET;
    else if (!tinybasic_strcmp(word, KEYWORD_IF) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_IF;
    else if (!tinybasic_strcmp(word, KEYWORD_THEN) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_THEN;
    else if (!tinybasic_strcmp(word, KEYWORD_GOTO) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_GOTO;
    else if (!tinybasic_strcmp(word, KEYWORD_GOSUB) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_GOSUB;
    else if (!tinybasic_strcmp(word, KEYWORD_RETURN) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_LET))
        return TOKEN_RETURN;
    else if (!tinybasic_strcmp(word, KEYWORD_END) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_END))
        return TOKEN_END;
    else if (!tinybasic_strcmp(word, KEYWORD_PRINT) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_PRINT))
        return TOKEN_PRINT;
    else if (!tinybasic_strcmp(word, KEYWORD_INPUT) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_INPUT))
        return TOKEN_INPUT;
    else if (!tinybasic_strcmp(word, KEYWORD_REM) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_REM))
        return TOKEN_REM;
    else if (!tinybasic_strcmp(word, KEYWORD_PEEK) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_PEEK))
        return TOKEN_PEEK;
    else if (!tinybasic_strcmp(word, KEYWORD_POKE) || !tinybasic_strcmp(word, DEFAULT_KEYWORD_POKE))
        return TOKEN_POKE;
    else if (strlen(word) >= 1)
        return TOKEN_VARIABLE; //variables should be more complicated
    else
        return TOKEN_WORD;
}

/*
 * Identify compound (multi-character) symbols.
 * Also identifies some single-character symbols that can form
 * the start of multi-character symbols.
 * params:
 *   TCHAR*   symbol   the symbol to identify
 * returns:
 *   TokenClass       the identification
 */
TokenClass identify_compound_symbol(const TCHAR* symbol) {
    if (!strcmp(symbol, _T("<>"))
        || !strcmp(symbol, _T("><")))
        return TOKEN_UNEQUAL;
    else if (!strcmp(symbol, _T("<")))
        return TOKEN_LESSTHAN;
    else if (!strcmp(symbol, _T("<=")))
        return TOKEN_LESSOREQUAL;
    else if (!strcmp(symbol, _T(">")))
        return TOKEN_GREATERTHAN;
    else if (!strcmp(symbol, _T(">=")))
        return TOKEN_GREATEROREQUAL;
    else
        return TOKEN_SYMBOL;
}

/*
 * Destructor for a TokenStream
 * params:
 *   TokenStream*   token_stream   the doomed token stream
 */
void destroy_ts(TokenStream* token_stream) {
    if (token_stream) {
        if (token_stream->data) {
            free(token_stream->data);
            token_stream->data = NULL;
        }
        free(token_stream);
    }
}

