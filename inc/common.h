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
#define ERROR_SUCCESSFUL			  _T("成功")
#define ERROR_INVALID_LINE_NUMBER	  _T("无效的行号")
#define ERROR_UNRECOGNISED_COMMAND    _T("未识别的命令")
#define ERROR_INVALID_VARIABLE        _T("无效的变量")
#define ERROR_INVALID_ASSIGNMENT      _T("无效的赋值")
#define ERROR_INVALID_EXPRESSION      _T("无效的表达式")
#define ERROR_MISSING                 _T("未找到 )")
#define ERROR_INVALID_PRINT_OUTPUT    _T("无效的打印语句")
#define ERROR_BAD_COMMAND_LINE        _T("错误的命令")
#define ERROR_FILE_NOT_FOUND          _T("文件未找到")
#define ERROR_INVALID_OPERATOR        _T("无效的操作符")
#define ERROR_THEN_EXPECTED           _T("缺少\"那么\"")
#define ERROR_UNEXPECTED_PARAMETER    _T("未预期的参数")
#define ERROR_RETURN_WITHOUT_GOSUB    _T("返回和调用子程序不匹配")
#define ERROR_DIVIDE_BY_ZERO          _T("除以零")
#define ERROR_OVERFLOW                _T("算数溢出")
#define ERROR_OUT_OF_MEMORY           _T("内存不足")
#define ERROR_TO_MANY_GOSUBS          _T("太多的子程序调用")

#define KEYWORD_END				  _T("结束")
#define KEYWORD_GOSUB			  _T("调用")
#define KEYWORD_GOTO			  _T("转到")
#define KEYWORD_IF				  _T("如果")
#define KEYWORD_INPUT			  _T("输入")
#define KEYWORD_LET				  _T("设定")
#define KEYWORD_PRINT			  _T("打印")
#define KEYWORD_REM				  _T("注释")
#define KEYWORD_RETURN			  _T("返回")
#define KEYWORD_THEN			  _T("那么")
#define KEYWORD_PEEK			  _T("读出")
#define KEYWORD_POKE			  _T("写入")

#define COMMAND_HELP			  _T("帮助")
#define COMMAND_SYSTEM			  _T("系统")
#define COMMAND_EXIT			  _T("退出")
#define COMMAND_RUN				  _T("运行")
#define COMMAND_LIST			  _T("列出")
#define TEXT_HELP				  _T("退出: 退出到操作系统\r\n运行: 运行程序\r\n列出: 列出程序清单\r\n")
#define TEXT_TOO_LONG			  _T("输入行太长\r\n")
#define TEXT_REPL_TITLE			  _T("中文版 Tiny Basic 2.0\r\n")

#define PROMPT_LINE_LABEL		  _T(", 行标号 %d")
#define PROMPT_SOURCE_LINE		  _T(", 源行号 %d 源列号 %d")

#define FORMATTER_IF_THEN		  _T("如果 %s%s%s 那么 %s")
#define FORMATTER_GOSUB			  _T("调用 %s")
#define FORMATTER_GOTO			  _T("转到 %s")
#define FORMATTER_LET			  _T("设定 %c=%s")
#define FORMATTER_PEEK			  _T("读出 %c, %s")
#define FORMATTER_POKE			  _T("写入 %s, %s")
#define FORMATTER_UNRECOGNISED    _T("未能识别语句.")

#define INTERPRETER_TYPE_ERROR	  _T("语句类型 %d 未实现.\n")

#define TINY_BASIC_USAGE		  _T("用法: %s [选项] [输入文件]\n")

#define TINY_BASIC_TBEXE_NOT_SET  _T("TBEXE 未设置.\n")
#define TINY_BASIC_RUNTIME_ERROR  _T("运行时错误: %s\n")
#define TINY_BASIC_PARSE_ERROR    _T("语法错误: %s\n")
#define TINY_BASIC_FILE_ERROR     _T("错误: 无法打开文件 %s\n")

#define TINY_BASIC_ENABLE		  _T("开启")
#define TINY_BASIC_DISABLE		  _T("禁用")

#define TINY_BASIC_OPTIONAL		  _T("可选")
#define TINY_BASIC_IMPLIED		  _T("隐含")
#define TINY_BASIC_MANDATORY      _T("强制")

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
