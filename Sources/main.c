#include <stdio.h>
#include <string.h>

#include <Lexer.h>
#include <Parser.h>
#include <Interpreter.h>
#include <JitCompiler.h>
#include <List.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


int main(int argc, char* argv[])
{
	//StringView input = sv_create("function a(b, c, d){ b + c * d; } if(x) { y  = 4; } else { y = 5; }");
	StringView input = sv_create("function fn1(){3+4*4;} function fn2(){10-3*2+4;fn1();} fn2();");
	LogInfo("Program input: '%s'\n", input.data);

	List* tokens = lexer_perform(input);
	PrintTokens(tokens);

	AstNode* tree = parser_perform(tokens);
	PrintAst(tree);

	//interpreter_perform(NULL, tree);
	fn_compiled_entry compiled_main = JIT_compile(tree);

	DWORD ex = 0;
	__try
	{
		unsigned long long result = compiled_main();
		LogInfo("Result from calling the compiled code: [%llu / 0x%llX]\n", result, result);
	}
	__except ((ex = GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER)
	{
		LogError("Failed to execute compiled code. Exception: [%ud / %X]\n", ex, ex);
	}

	ast_tree_free(tree);
	list_free(tokens);

	return 0;
}