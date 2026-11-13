#include <stdio.h>
#include <string.h>

#include <Lexer.h>
#include <Parser.h>
#include <JitCompiler.h>
#include <JitException.h>
#include <List.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


int main(int argc, char* argv[])
{
	//StringView input = sv_create("function a(b, c, d){ b + c * d; } if(x) { y  = 4; } else { y = 5; }");
	//StringView input = sv_create("function fn1(){3+4*4;} function fn2(){x = 10; y = fn1(); x-3*2+4 + y;} z = fn2(); z;");
	//StringView input = sv_create("x=3; x = x + 3;");
	//StringView input = sv_create("function fn1(x,y){ x+y; }  fn1(3,5*2);");
	//StringView input = sv_create("i = 0; while(i != 10) {i = i + 1;} return i;");
	//StringView input = sv_create("function a(i) { i=3; return 2; i=5; } a(1);");
	//StringView input = sv_create("a = 0; for(i = 3; i < 10; i = i + 3){ a = a + 1; } return a;");
	//StringView input = sv_create("i = 3; if(2 == 1 || 1 == 1) { i = 5;} return i; ");
	//StringView input = sv_create("i = 3; return (-i); ");
	StringView input = sv_create("13 % 3;");


	LogInfo("Program input: '%s'\n", input.data);

	List* tokens = lexer_perform(input);
	PrintTokens(tokens);

	AstNode* tree = parser_perform(tokens);
	PrintAst(tree);

	//interpreter_perform(NULL, tree);
	fn_compiled_entry compiled_main = NULL;
	size_t compiled_size = JIT_compile(tree, &compiled_main);

	DWORD ex = 0;
	__try
	{
		InstallExceptionHandler(compiled_main, compiled_size);
		unsigned long long result = compiled_main();
		LogInfo("Result from calling the compiled code: [%llu / 0x%llX]\n", result, result);
	}
	__except ((ex = GetExceptionCode()), EXCEPTION_EXECUTE_HANDLER)
	{
		LogError("Failed to execute compiled code. Exception: [%ud / %X]\n", ex, ex);
	}
	
	RemoveExceptionHandler();

	ast_tree_free(tree);
	list_free(tokens);

	return 0;
}