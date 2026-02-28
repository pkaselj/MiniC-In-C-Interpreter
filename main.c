#include <stdio.h>

#include "Lexer.h"

int main(int argc, char* argv[])
{
	StringView input = sv_create("\"Test\"");
	TokenList* list = lexer_perform(&input);


	return 0;
}