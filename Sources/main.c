#include <stdio.h>
#include <string.h>

#include <Lexer.h>
#include <Parser.h>
#include <List.h>


int main(int argc, char* argv[])
{
	StringView input = sv_create("function a(b, c, d){ b + c * d; } if(x) { y  = 4; } else { y = 5; }");
	printf("Program input: '%s'\n", input.data);

	List* tokens = lexer_perform(input);
	PrintTokens(tokens);

	AstNode* tree = parser_perform(tokens);
	PrintAst(tree);

	return 0;
}