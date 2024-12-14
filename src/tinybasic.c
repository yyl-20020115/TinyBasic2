/*
 * Tiny BASIC
 * Interpreter and Compiler Main Program
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 04-Aug-2019
 */


 /* included headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "common.h"
#include "options.h"
#include "errors.h"
#include "parser.h"
#include "statement.h"
#include "interpret.h"
#include "formatter.h"
#include "generatec.h"


#define TINY_BASIC_TARGET		  _T("$(TARGET)")
#define TINY_BASIC_SOURCE		  _T("$(SOURCE)")

/* static variables */
static TCHAR* input_filename = NULL; /* name of the input file */
static enum { /* action to take with parsed program */
	OUTPUT_INTERPRET, /* interpret the program */
	OUTPUT_LST, /* output a formatted listing */
	OUTPUT_C, /* output a C program */
	OUTPUT_EXE /* output an executable */
} output = OUTPUT_INTERPRET;


/*
 * Level 2 Routines
 */


 /*
  * Set line number option
  * params:
  *   TCHAR*   option   the option supplied on the command line
  */
static void set_line_numbers(TCHAR* option, ErrorHandler* errors, LanguageOptions* loptions) {
	if (!strncmp(TINY_BASIC_OPTIONAL, option, strlen(option)))
		loptions->set_line_numbers(loptions, LINE_NUMBERS_OPTIONAL);
	else if (!strncmp(TINY_BASIC_IMPLIED, option, strlen(option)))
		loptions->set_line_numbers(loptions, LINE_NUMBERS_IMPLIED);
	else if (!strncmp(TINY_BASIC_MANDATORY, option, strlen(option)))
		loptions->set_line_numbers(loptions, LINE_NUMBERS_MANDATORY);
	else
		errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
}

/*
 * Set line number limit
 * params:
 *   TCHAR*   option   the option supplied on the command line
 */
static void set_line_limit(TCHAR* option, ErrorHandler* errors, LanguageOptions* loptions) {
	int limit; /* the limit contained in the option */
	if (sscanf(option, _T("%d"), &limit))
		loptions->set_line_limit(loptions, limit);
	else
		errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
}

/*
 * Set comment option
 * params:
 *   TCHAR*   option   the option supplied on the command line
 */
static void set_comments(TCHAR* option, ErrorHandler* errors, LanguageOptions* loptions) {
	if (!strncmp(TINY_BASIC_ENABLE, option, strlen(option)))
		loptions->set_comments(loptions, COMMENTS_ENABLED);
	else if (!strncmp(TINY_BASIC_DISABLE, option, strlen(option)))
		loptions->set_comments(loptions, COMMENTS_DISABLED);
	else
		errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
}

/*
 * Set the output options
 * params:
 *   TCHAR*   option   the option supplied on the command line
 */
static void set_output(TCHAR* option, ErrorHandler* errors, LanguageOptions* loptions) {
	if (!strcmp(_T("lst"), option))
		output = OUTPUT_LST;
	else if (!strcmp(_T("c"), option))
		output = OUTPUT_C;
	else if (!strcmp(_T("exe"), option))
		output = OUTPUT_EXE;
	else
		errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
}

/*
 * Set the GOSUB stack limit option
 * params:
 *   TCHAR*   option   the option supplied on the command line
 */
static void set_gosub_limit(TCHAR* option, ErrorHandler* errors, LanguageOptions* loptions) {
	int limit; /* the limit contained in the option */
	if (sscanf(option, _T("%d"), &limit))
		loptions->set_gosub_limit(loptions, limit);
	else
		errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
}


/*
 * Level 1 Routines
 */


 /*
  * Process the command line options
  * params:
  *   int     argc   number of arguments on the command line
  *   TCHAR**  argv   the arguments
  */
static void set_options(int argc, TCHAR** argv, ErrorHandler* errors, LanguageOptions* loptions) {

	/* local variables */
	int argn; /* argument number count */

	/* loop through all parameters */
	for (argn = 1; argn < argc && !errors->get_code(errors); ++argn) {

		/* scan for line number options */
		if (!strncmp(argv[argn], _T("-n"), 2))
			set_line_numbers(&argv[argn][2], errors, loptions);
		else if (!strncmp(argv[argn], _T("--line-numbers="), 15))
			set_line_numbers(&argv[argn][15], errors, loptions);

		/* scan for line number limit */
		else if (!strncmp(argv[argn], _T("-N"), 2))
			set_line_limit(&argv[argn][2], errors, loptions);
		else if (!strncmp(argv[argn], _T("--line-limit="), 13))
			set_line_limit(&argv[argn][13], errors, loptions);

		/* scan for comment option */
		else if (!strncmp(argv[argn], _T("-o"), 2))
			set_comments(&argv[argn][2], errors, loptions);
		else if (!strncmp(argv[argn], _T("--comments="), 11))
			set_comments(&argv[argn][11], errors, loptions);

		/* scan for output option */
		else if (!strncmp(argv[argn], _T("-O"), 2))
			set_output(&argv[argn][2], errors, loptions);
		else if (!strncmp(argv[argn], _T("--output="), 9))
			set_output(&argv[argn][9], errors, loptions);

		/* scan for gosub stack limit */
		else if (!strncmp(argv[argn], _T("-g"), 2))
			set_gosub_limit(&argv[argn][2], errors, loptions);
		else if (!strncmp(argv[argn], _T("--gosub-limit="), 14))
			set_gosub_limit(&argv[argn][14], errors, loptions);
		else if (!strncmp(argv[argn], _T("--help"), 6)) {

		}
		/* accept filename */
		else if (!input_filename)
			input_filename = argv[argn];

		/* raise an error upon illegal option */
		else
			errors->set_code(errors, E_BAD_COMMAND_LINE, 0, 0, 0);
	}
}

/*
 * Output a formatted program listing
 * params:
 *   ProgramNode*   program   the program to output
 */
static void output_lst(ProgramNode* program, ErrorHandler* errors, LanguageOptions* loptions) {

	/* local variables */
	FILE* output; /* the output file */
	TCHAR* output_filename; /* the output filename */
	Formatter* formatter; /* the formatter object */

	/* ascertain the output filename */
	output_filename = _malloc(strlen(input_filename) + 5);
	if (output_filename) {

		/* open the output file */
		snprintf(output_filename, strlen(input_filename) + 5, _T("%s.lst"), input_filename);
		if ((output = fopen(output_filename, _T("w")))) {

			/* write to the output file */
			formatter = new_Formatter(errors);
			if (formatter) {
				formatter->generate(formatter, program);
				if (formatter->output)
					fprintf(output, _T("%s"), formatter->output);
				formatter->destroy(formatter);
			}
			fclose(output);
		}

		/* deal with errors */
		else
			errors->set_code(errors, E_FILE_NOT_FOUND, 0, 0, 0);

		/* free the output filename */
		free(output_filename);
	}

	/* deal with out of memory error */
	else
		errors->set_code(errors, E_MEMORY, 0, 0, 0);
}

/*
 * Output a C source file
 * params:
 *   ProgramNode*   program   the parsed program
 */
static void output_c(ProgramNode* program, ErrorHandler* errors, LanguageOptions* loptions) {

	/* local variables */
	FILE* output; /* the output file */
	TCHAR* output_filename; /* the output filename */
	CProgram* c_program; /* the C program */

	/* open the output file */
	output_filename = _malloc(strlen(input_filename) + 5);
	if (output_filename != NULL)
		snprintf(output_filename, strlen(input_filename) + 5, _T("%s.c"), input_filename);
	if (output_filename != NULL && (output = fopen(output_filename, _T("w")))) {

		/* write to the output file */
		c_program = new_CProgram(errors, loptions);
		if (c_program) {
			c_program->generate(c_program, program);
			if (c_program->c_output)
				fprintf(output, _T("%s"), c_program->c_output);
			c_program->destroy(c_program);
		}
		fclose(output);
	}

	/* deal with errors */
	else
		errors->set_code(errors, E_FILE_NOT_FOUND, 0, 0, 0);

	/* clean up allocated memory */
	free(output_filename);
}

/*
 * Invoke a compiler to turn a C source file into an executable
 * params:
 *   TCHAR*   basic_filename   The BASIC program's name
 */
static void output_exe(TCHAR* command, TCHAR* basic_filename) {

	/* local variables */
	TCHAR
		c_filename[256], /* the name of the C source */
		exe_filename[256], /* the base name of the executable */
		final_command[1024], /* the constructed compiler command */
		* ext, /* position of extension character '.' in filename */
		* src, /* source pointer for string copying */
		* dst; /* destination pointer for string copying */

	/* work out the C and EXE filenames */
	snprintf(c_filename, 256, _T("%s.c"), basic_filename);
	strcpy(exe_filename, basic_filename);
	if ((ext = strchr(exe_filename, _T('.'))))
		*ext = _T('\0');
	else
		strcat(exe_filename, _T(".out"));

	/* build the compiler command */
	src = command;
	dst = final_command;
	while (*src) {
		if (!strncmp(src, TINY_BASIC_TARGET, strlen(TINY_BASIC_TARGET))) {
			strcpy(dst, exe_filename);
			dst += strlen(exe_filename);
			src += strlen(TINY_BASIC_TARGET);
		}
		else if (!strncmp(src, TINY_BASIC_SOURCE, strlen(TINY_BASIC_SOURCE))) {
			strcpy(dst, c_filename);
			dst += strlen(c_filename);
			src += strlen(TINY_BASIC_SOURCE);
		}
		else
			*(dst++) = *(src++);
	}
	*dst = '\0';

	/* run the compiler command */
	system(final_command);
}


/*
 * Top Level Routines
 */
static TCHAR* trim_line(TCHAR* line, size_t line_length) {
	if (line != NULL && line_length >= 1) {
		size_t len = strlen(line);
		if (len > 0) {
			int i = 0, p = 0;
			while (i < len && line[i++] == _T(' '));
			i--;
			if (i > 0) {
				for (int q = i; p < len - q;) line[p++] = line[i++];
				i = p - 1;
				for (; p < len; p++) line[p] = _T('\0');
			}
			if (i == 0) i = (int)strlen(line) - 1;
			while ((line[i] == _T(' ') || line[i] == _T('\n')))
				i--;
			if (i >= 0 && i + 2 < line_length) {
				line[i + 1] = _T('\n');
				line[i + 2] = _T('\0');
			}
		}
	}
	return line;
}
static int run(TCHAR* text, int length, LanguageOptions* loptions) {
	ErrorHandler* errors;
	ProgramNode* program; /* the parsed program */
	ErrorCode code; /* error returned */
	Parser* parser; /* parser object */
	Interpreter* interpreter; /* interpreter object */
	if (NULL != (errors = new_ErrorHandler()))
	{
		if (NULL != (parser = new_BufferParser(errors, loptions, text, length)))
		{
			if (NULL != (program = parser->parse(parser)))
			{
				if (NULL != (interpreter = new_Interpreter(errors, loptions)))
				{
					interpreter->interpret(interpreter, program);
					if ((code = errors->get_code(errors))) {
						TCHAR* error_text = errors->get_text(errors);
						printf(TINY_BASIC_RUNTIME_ERROR, error_text);
						free(error_text);
					}
					interpreter->destroy(interpreter);
				}
				program_destroy(program);
			}
			parser->destroy(parser);
		}
	}
	return 0;
}
static int tiny_basic_repl(int line_length, LanguageOptions* loptions) {
	TCHAR* line_buffer = (TCHAR*)_malloc(line_length);
	if (line_buffer == NULL) return -1;
	TCHAR** list = (TCHAR**)malloc(sizeof(TCHAR*) * REPL_MAX_LINES);
	if (list == NULL) return -1;
	memset(list, 0, sizeof(TCHAR*) * REPL_MAX_LINES);
	//show title
	printf(TEXT_REPL_TITLE);

	ProgramNode* program; /* the parsed program */
	ErrorCode code; /* error returned */
	Parser* parser; /* parser object */
	Interpreter* interpreter; /* interpreter object */

	TCHAR* error_text; /* error text message */
	int last_line_number = 0;
	int ch;
	int count;
	ErrorHandler* errors = new_ErrorHandler();
	if (NULL != (interpreter = new_Interpreter(errors, loptions)))
	{
		while (1) {
			memset(line_buffer, 0, sizeof(TCHAR) * line_length);
			for (count = 0; count < line_length; count++) {
				ch = getchar();
				line_buffer[count] = ch;
				if (ch == _T('\n')) {
					break;
				}
			}
			if (count == line_length) {
				printf(TEXT_TOO_LONG);
				continue;
			}
#ifndef USE_EMBEDDED
			//you can quit if you're not in embedeed mode
			if (0 == tinybasic_strcmp(line_buffer, COMMAND_SYSTEM)
				|| 0 == tinybasic_strcmp(line_buffer, COMMAND_EXIT)
				|| 0 == tinybasic_strcmp(line_buffer, DEFAULT_COMMAND_SYSTEM)
				|| 0 == tinybasic_strcmp(line_buffer, DEFAULT_COMMAND_EXIT)
				) {
				break;
			}
#endif
			int ln = 0, lm = 0;
			char* ps = 0, * bs = 0;
			line_buffer = trim_line(line_buffer, line_length);
			//you can get help with command HELP
			if (0 == tinybasic_strcmp(line_buffer, COMMAND_HELP)
				|| 0 == tinybasic_strcmp(line_buffer, DEFAULT_COMMAND_HELP)
				|| line_buffer[0]=='?') {
				printf(TEXT_HELP);
			}
			//you can run the program with command RUN
			else if (0 == tinybasic_strcmp(line_buffer, COMMAND_RUN)
				|| 0 == tinybasic_strcmp(line_buffer, DEFAULT_COMMAND_RUN)) {
				//RUN
				//RUN LINE-NUMBER
				if ((ps = strchr(line_buffer, _T(' '))) != 0)
					if((1 == sscanf(ps + 1, _T("%d"), &ln)
						&& ln >= 0 && ln < REPL_MAX_LINES)) {
				}
				int total_buffer_length = 1;
				for (int t = ln; t < REPL_MAX_LINES; t++) {
					total_buffer_length += (list[t] == NULL ? 0 : (int)strlen(list[t]));
				}
				TCHAR* total_buffer = _malloc(total_buffer_length);
				if (total_buffer != NULL) {
					memset(total_buffer, 0, total_buffer_length * sizeof(TCHAR));
					for (int t = ln; t < REPL_MAX_LINES; t++) {
						if (list[t] != NULL) {
							strcat(total_buffer, list[t]);
						}
					}
					run(total_buffer, total_buffer_length - 1, loptions);

					free(total_buffer);

				}
			}
			//you can list the program with command LIST
			else if (0 == tinybasic_strcmp(line_buffer, COMMAND_LIST)
				|| 0 == tinybasic_strcmp(line_buffer, DEFAULT_COMMAND_LIST)) {
				//LIST
				//LIST LINE-NUMBER
				//LIST LINE-NUMBER-START LINE-NUMBER-END
				if (strlen(line_buffer) == 5) {
					lm = REPL_MAX_LINES;
				}
				else if (((ps = strchr(line_buffer, _T(' '))) != 0)
					&& ((bs = strchr(line_buffer, _T('-'))) - line_buffer) > 4
					&& (1 == sscanf(ps + 1, _T("%d"), &ln))
					&& (1 == sscanf(bs + 1, _T("%d"), &lm))) {
				}
				else if (((ps = strchr(line_buffer, _T(' ')))!=0)
					&& (1 == sscanf(ps + 1, _T("%d"), &ln))) {
					lm = REPL_MAX_LINES;
				}

				ln = ln < 0 || ln>REPL_MAX_LINES ? 0 : ln;
				lm = lm <= ln ? ln : lm;
				lm = lm >= REPL_MAX_LINES ? lm = REPL_MAX_LINES - 1 : lm;
				for (int t = ln; t <= lm; t++) {
					if (list[t] == NULL) continue;
					printf(list[t]);
				}
			}
			//
			else if (1 == sscanf(line_buffer, _T("%d "), &ln)
				&& ln >= 0 && ln < REPL_MAX_LINES) {
				//you can edit a line with line number or append a line to last line number
				//or redirect current line with a new line number
				if (list[ln] != NULL) free(list[ln]);
				list[ln] = _strdup(line_buffer);
			}
			else if (line_buffer[0] == _T('?')) //show one line
			{
				int ln = 0;
				if (1 == sscanf(line_buffer + 1, _T("%d"), &ln)
					&& ln >= 0 && ln < REPL_MAX_LINES
					&& list[ln] != NULL) {
					printf(list[ln]);
				}
			}
			else if (line_buffer[0] == _T('/')) //remove one line
			{
				int ln = 0;
				if (1 == sscanf(line_buffer + 1, _T("%d"), &ln)
					&& ln >= 0 && ln < REPL_MAX_LINES
					&& list[ln] != NULL) {
					free(list[ln]);
					list[ln] = NULL;
				}
			}
			else {
				//or run the instant statement
				if (NULL != (parser = new_BufferParser(errors, loptions, line_buffer, count))) {
					if (NULL != (program = parser->parse(parser)))
					{
						interpreter->interpret(interpreter, program);
						if ((code = errors->get_code(errors))) {
							error_text = errors->get_text(errors);
							printf(TINY_BASIC_RUNTIME_ERROR, error_text);
							free(error_text);
						}
						program_destroy(program);
					}
					parser->destroy(parser);
				}
			}
		}
		interpreter->destroy(interpreter);
	}
	free(line_buffer);
	return 0;
}

/*
 * Main Program
 * params:
 *   int     argc   number of arguments on the command line
 *   TCHAR**  argv   the arguments
 * returns:
 *   int            any error code from processing/running the program
 */
#ifdef USE_EMBEDDED
int main()
{
	tiny_basic_repl(REPL_LINE_LENGTH, loptions);
	while (1);
}
#else
int main(int argc, TCHAR** argv)
{
	setlocale(LC_ALL, "chs");

	/* local variables */
	FILE* input; /* input file */
	ProgramNode* program; /* the parsed program */
	ErrorCode code; /* error returned */
	Parser* parser; /* parser object */
	Interpreter* interpreter; /* interpreter object */
	TCHAR
		* error_text, /* error text message */
		* command; /* command for compilation */
	int ret = 0;
	/* interpret the command line arguments */
	ErrorHandler* errors; /* universal error handler */
	LanguageOptions* loptions; /* language options */

	errors = new_ErrorHandler();
	loptions = new_LanguageOptions();
	set_options(argc, argv, errors, loptions);

	/* give usage if filename not given */
	if (input_filename == NULL) {
		//use repl
		ret = tiny_basic_repl(REPL_LINE_LENGTH, loptions);
		errors->destroy(errors);
		loptions->destroy(loptions);
		return ret;
	}

	/* otherwise attempt to open the file */
	if (!(input = fopen(input_filename, _T("r")))) {
		printf(TINY_BASIC_FILE_ERROR, input_filename);
		errors->destroy(errors);
		loptions->destroy(loptions);
		return E_FILE_NOT_FOUND;
	}

	/* get the parse tree */
	parser = new_Parser(errors, loptions, input);
	program = parser->parse(parser);
	parser->destroy(parser);
	fclose(input);

	/* deal with errors */
	if ((code = errors->get_code(errors))) {
		error_text = errors->get_text(errors);
		printf(TINY_BASIC_PARSE_ERROR, error_text);
		free(error_text);
		loptions->destroy(loptions);
		errors->destroy(errors);
		return code;
	}

	/* perform the desired action */
	switch (output) {
	case OUTPUT_INTERPRET:
		interpreter = new_Interpreter(errors, loptions);
		interpreter->interpret(interpreter, program);
		interpreter->destroy(interpreter);
		if ((code = errors->get_code(errors))) {
			error_text = errors->get_text(errors);
			printf(TINY_BASIC_RUNTIME_ERROR, error_text);
			free(error_text);
		}
		break;
	case OUTPUT_LST:
		output_lst(program, errors, loptions);
		break;
	case OUTPUT_C:
		output_c(program, errors, loptions);
		break;
	case OUTPUT_EXE:
		if ((command = getenv(_T("TBEXE"))))
		{
			output_c(program, errors, loptions);
			output_exe(command, input_filename);
		}
		else
			printf(TINY_BASIC_TBEXE_NOT_SET);
		break;
	}

	/* clean up and return success */
	program_destroy(program);
	loptions->destroy(loptions);
	errors->destroy(errors);
	return ret;

}
#endif