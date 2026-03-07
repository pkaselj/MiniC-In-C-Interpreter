#include <stdio.h>
#include <string.h>

#include <Lexer.h>
#include <Parser.h>
#include <Interpreter.h>
#include <List.h>


int main(int argc, char* argv[])
{
	//StringView input = sv_create("function a(b, c, d){ b + c * d; } if(x) { y  = 4; } else { y = 5; }");
	StringView input = sv_create("3*5+15-2;");
	printf("Program input: '%s'\n", input.data);

	List* tokens = lexer_perform(input);
	PrintTokens(tokens);

	AstNode* tree = parser_perform(tokens);
	PrintAst(tree);

	interpreter_perform(NULL, tree);

	return 0;
}