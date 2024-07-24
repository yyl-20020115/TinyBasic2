/*
 * Tiny BASIC Interpreter and Compiler Project
 * Common service routines module
 *
 * Released as Public Domain by Damian Gareth Walker 2019
 * Created: 20-Sep-2019
 */


#ifndef __COMMON_H__
#define __COMMON_H__

#define REPL_LINE_LENGTH   256
#define REPL_MAX_LINES     4096

#ifndef USE_DEFAULTS
#define USE_DEFAULTS
#endif

#ifndef USE_EMBEDDED
//#define USE_EMBEDDED
#endif

#ifndef USE_WCHAR
#define USE_WCHAR
#endif

#ifndef USE_CHINESE
#define USE_CHINESE
#endif


#ifdef  USE_WCHAR
#define LAST_ANSI 0xff
#define _N(n) ((n)*2)
#define _malloc(n) malloc(_N(n))
#define _realloc(a,n) realloc(a,_N(n))

#define TCHAR wchar_t
#define _T(x) L##x
#define _strdup _wcsdup
#define strcmp wcscmp
#define strlen wcslen
#define snprintf swprintf
#define strcat wcscat
#define strcpy wcscpy
#define strncpy wcsncpy
#define strncmp wcsncmp
#define getenv(n) _wgetenv(n)
#define printf wprintf
#define fprintf fwprintf
#define fgetc fgetwc
#define ungetc ungetwc
#define getchar getwchar
#define system _wsystem
#define fopen _wfopen
#define strchr wcschr
#define sscanf swscanf
#define atoi _wtoi
#define main wmain
#else
#define TCHAR char
#define _T(x) x
#define _malloc malloc
#define _realloc realloc
#endif

#ifdef  USE_DEFAULTS

#define DEFAULT_KEYWORD_END				  _T("END")
#define DEFAULT_KEYWORD_GOSUB			  _T("GOSUB")
#define DEFAULT_KEYWORD_GOTO			  _T("GOTO")
#define DEFAULT_KEYWORD_IF				  _T("IF")
#define DEFAULT_KEYWORD_INPUT			  _T("INPUT")
#define DEFAULT_KEYWORD_LET				  _T("LET")
#define DEFAULT_KEYWORD_PRINT			  _T("PRINT")
#define DEFAULT_KEYWORD_REM				  _T("REM")
#define DEFAULT_KEYWORD_RETURN			  _T("RETURN")
#define DEFAULT_KEYWORD_THEN			  _T("THEN")
#define DEFAULT_KEYWORD_PEEK			  _T("PEEK")
#define DEFAULT_KEYWORD_POKE			  _T("POKE")

#define DEFAULT_COMMAND_HELP			  _T("HELP")
#define DEFAULT_COMMAND_SYSTEM			  _T("SYSTEM")
#define DEFAULT_COMMAND_EXIT 			  _T("EXIT")
#define DEFAULT_COMMAND_RUN				  _T("RUN")
#define DEFAULT_COMMAND_LIST			  _T("LIST")

#else
#define DEFAULT_KEYWORD_END				  KEYWORD_END
#define DEFAULT_KEYWORD_GOSUB			  KEYWORD_GOSUB
#define DEFAULT_KEYWORD_GOTO			  KEYWORD_GOTO
#define DEFAULT_KEYWORD_IF				  KEYWORD_IF
#define DEFAULT_KEYWORD_INPUT			  KEYWORD_INPUT
#define DEFAULT_KEYWORD_LET				  KEYWORD_LET
#define DEFAULT_KEYWORD_PRINT			  KEYWORD_PRINT
#define DEFAULT_KEYWORD_REM				  KEYWORD_REM
#define DEFAULT_KEYWORD_RETURN			  KEYWORD_RETURN
#define DEFAULT_KEYWORD_THEN			  KEYWORD_THEN

#define DEFAULT_COMMAND_HELP			  COMMAND_HELP
#define DEFAULT_COMMAND_SYSTEM			  COMMAND_SYSTEM
#define DEFAULT_COMMAND_EXIT 			  COMMAND_EXIT
#define DEFAULT_COMMAND_RUN				  COMMAND_RUN
#define DEFAULT_COMMAND_LIST			  COMMAND_LIST

#endif


#ifdef USE_CHINESE
#define ERROR_SUCCESSFUL			  _T("�ɹ�")
#define ERROR_INVALID_LINE_NUMBER	  _T("��Ч���к�")
#define ERROR_UNRECOGNISED_COMMAND    _T("δʶ�������")
#define ERROR_INVALID_VARIABLE        _T("��Ч�ı���")
#define ERROR_INVALID_ASSIGNMENT      _T("��Ч�ĸ�ֵ")
#define ERROR_INVALID_EXPRESSION      _T("��Ч�ı��ʽ")
#define ERROR_MISSING                 _T("δ�ҵ� )")
#define ERROR_INVALID_PRINT_OUTPUT    _T("��Ч�Ĵ�ӡ���")
#define ERROR_BAD_COMMAND_LINE        _T("���������")
#define ERROR_FILE_NOT_FOUND          _T("�ļ�δ�ҵ�")
#define ERROR_INVALID_OPERATOR        _T("��Ч�Ĳ�����")
#define ERROR_THEN_EXPECTED           _T("ȱ��\"��ô\"")
#define ERROR_UNEXPECTED_PARAMETER    _T("δԤ�ڵĲ���")
#define ERROR_RETURN_WITHOUT_GOSUB    _T("���غ͵����ӳ���ƥ��")
#define ERROR_DIVIDE_BY_ZERO          _T("������")
#define ERROR_OVERFLOW                _T("�������")
#define ERROR_OUT_OF_MEMORY           _T("�ڴ治��")
#define ERROR_TO_MANY_GOSUBS          _T("̫����ӳ������")

#define KEYWORD_END				  _T("����")
#define KEYWORD_GOSUB			  _T("����")
#define KEYWORD_GOTO			  _T("ת��")
#define KEYWORD_IF				  _T("���")
#define KEYWORD_INPUT			  _T("����")
#define KEYWORD_LET				  _T("�趨")
#define KEYWORD_PRINT			  _T("��ӡ")
#define KEYWORD_REM				  _T("ע��")
#define KEYWORD_RETURN			  _T("����")
#define KEYWORD_THEN			  _T("��ô")
#define KEYWORD_PEEK			  _T("����")
#define KEYWORD_POKE			  _T("д��")

#define COMMAND_HELP			  _T("����")
#define COMMAND_SYSTEM			  _T("ϵͳ")
#define COMMAND_EXIT			  _T("�˳�")
#define COMMAND_RUN				  _T("����")
#define COMMAND_LIST			  _T("�г�")
#define TEXT_HELP				  _T("�˳�: �˳�������ϵͳ\r\n����: ���г���\r\n�г�: �г������嵥\r\n")
#define TEXT_TOO_LONG			  _T("������̫��\r\n")
#define TEXT_REPL_TITLE			  _T("���İ� Tiny Basic 2.0\r\n")

#define PROMPT_LINE_LABEL		  _T(", �б�� %d")
#define PROMPT_SOURCE_LINE		  _T(", Դ�к� %d Դ�к� %d")

#define FORMATTER_IF_THEN		  _T("��� %s%s%s ��ô %s")
#define FORMATTER_GOSUB			  _T("���� %s")
#define FORMATTER_GOTO			  _T("ת�� %s")
#define FORMATTER_LET			  _T("�趨 %c=%s")
#define FORMATTER_PEEK			  _T("���� %c, %s")
#define FORMATTER_POKE			  _T("д�� %s, %s")
#define FORMATTER_UNRECOGNISED    _T("δ��ʶ�����.")

#define INTERPRETER_TYPE_ERROR	  _T("������� %d δʵ��.\n")

#define TINY_BASIC_USAGE		  _T("�÷�: %s [ѡ��] [�����ļ�]\n")

#define TINY_BASIC_TBEXE_NOT_SET  _T("TBEXE δ����.\n")
#define TINY_BASIC_RUNTIME_ERROR  _T("����ʱ����: %s\n")
#define TINY_BASIC_PARSE_ERROR    _T("�﷨����: %s\n")
#define TINY_BASIC_FILE_ERROR     _T("����: �޷����ļ� %s\n")

#define TINY_BASIC_ENABLE		  _T("����")
#define TINY_BASIC_DISABLE		  _T("����")

#define TINY_BASIC_OPTIONAL		  _T("��ѡ")
#define TINY_BASIC_IMPLIED		  _T("����")
#define TINY_BASIC_MANDATORY      _T("ǿ��")

#else
#define ERROR_SUCCESSFUL			  _T("Successful")
#define ERROR_INVALID_LINE_NUMBER	  _T("Invalid line number")
#define ERROR_UNRECOGNISED_COMMAND    _T("Unrecognised command")
#define ERROR_INVALID_VARIABLE        _T("Invalid variable")
#define ERROR_INVALID_ASSIGNMENT      _T("Invalid assignment")
#define ERROR_INVALID_EXPRESSION      _T("Invalid expression")
#define ERROR_MISSING                 _T("Missing )")
#define ERROR_INVALID_PRINT_OUTPUT    _T("Invalid PRINT output")
#define ERROR_BAD_COMMAND_LINE        _T("Bad command line")
#define ERROR_FILE_NOT_FOUND          _T("File not found")
#define ERROR_INVALID_OPERATOR        _T("Invalid operator")
#define ERROR_THEN_EXPECTED           _T("THEN expected")
#define ERROR_UNEXPECTED_PARAMETER    _T("Unexpected parameter")
#define ERROR_RETURN_WITHOUT_GOSUB    _T("RETURN without GOSUB")
#define ERROR_DIVIDE_BY_ZERO          _T("Divide by zero")
#define ERROR_OVERFLOW                _T("Overflow")
#define ERROR_OUT_OF_MEMORY           _T("Out of memory")
#define ERROR_TO_MANY_GOSUBS          _T("Too many gosubs")
								  
#define KEYWORD_END				  _T("END")
#define KEYWORD_GOSUB			  _T("GOSUB")
#define KEYWORD_GOTO			  _T("GOTO")
#define KEYWORD_IF				  _T("IF")
#define KEYWORD_INPUT			  _T("INPUT")
#define KEYWORD_LET				  _T("LET")
#define KEYWORD_PRINT			  _T("PRINT")
#define KEYWORD_REM				  _T("REM")
#define KEYWORD_RETURN			  _T("RETURN")
#define KEYWORD_THEN			  _T("THEN")
#define KEYWORD_PEEK			  _T("PEEK")
#define KEYWORD_POKE			  _T("POKE")

#define COMMAND_HELP			  _T("HELP")
#define COMMAND_SYSTEM			  _T("SYSTEM")
#define COMMAND_EXIT 			  _T("EXIT")
#define COMMAND_RUN				  _T("RUN")
#define COMMAND_LIST			  _T("LIST")

#define TEXT_HELP				  _T("SYSTEM: Exit to OS\r\nRUN: Run the program\r\nLIST: List the program.")
#define TEXT_TOO_LONG			  _T("Input line is too long")
#define TEXT_REPL_TITLE			  _T("Tiny Basic V2.0\r\n")
#define PROMPT_LINE_LABEL		  _T(", line label %d")
#define PROMPT_SOURCE_LINE		  _T(", source line %d, source column %d")
								  
#define FORMATTER_IF_THEN		  _T("IF %s%s%s THEN %s")
#define FORMATTER_GOSUB			  _T("GOSUB %s")
#define FORMATTER_GOTO			  _T("GOTO %s")
#define FORMATTER_LET			  _T("LET %c=%s")
#define FORMATTER_UNRECOGNISED    _T("Unrecognised statement.")
#define FORMATTER_PEEK			  _T("PEEK %s, %s")
#define FORMATTER_POKE			  _T("POKE %c, %s")

#define GENERATOR_INCLUDE_STDIO   _T("#include <stdio.h>\n")
#define GENERATOR_INCLUDE_STDLIB  _T("#include <stdlib.h>\n")
#define GENERATOR_DEFINE_ERROR    _T("#define E_RETURN_WITHOUT_GOSUB %d\n")

#define INTERPRETER_TYPE_ERROR	  _T("Statement type %d not implemented.\n")

#define TINY_BASIC_USAGE		  _T("Usage: %s [OPTIONS] [INPUT-FILE]\n")

#define TINY_BASIC_TBEXE_NOT_SET  _T("TBEXE not set.\n")
#define TINY_BASIC_RUNTIME_ERROR  _T("Runtime error: %s\n")
#define TINY_BASIC_PARSE_ERROR    _T("Parse error: %s\n")
#define TINY_BASIC_FILE_ERROR     _T("Error: cannot open file %s\n")

#define TINY_BASIC_ENABLE		  _T("enabled")
#define TINY_BASIC_DISABLE		  _T("disabled")

#define TINY_BASIC_OPTIONAL		  _T("optional")
#define TINY_BASIC_IMPLIED		  _T("implied")
#define TINY_BASIC_MANDATORY      _T("mandatory")

#endif


/*
 * Function Declarations
 */


/*
 * Portable case-insensitive comparison
 * params:
 *   TCHAR*   a   string to compare
 *   TCHAR*   b   string to compare to
 * returns:
 *   int         -1 if a<b, 0 if a==b, 1 if a>b
 */
int tinybasic_strcmp (const TCHAR *a, const TCHAR *b);


#endif
