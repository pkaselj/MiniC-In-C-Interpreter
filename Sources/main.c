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
	//StringView input = sv_create("function fn1(){3+4*4;} function fn2(){x = 10; y = fn1(); x-3*2+4 + y;} z = fn2(); z;");
	//StringView input = sv_create("x=3; x = x + 3;");
	StringView input = sv_create("function fn1(x,y){ x+y; }  fn1(3,5*2);");
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