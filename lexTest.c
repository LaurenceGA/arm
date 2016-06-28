#include <stdio.h>

#include "headers/lexer.h"

/* arm is a simple interpreted language */

int main(int argc, char *argv[]) {
	FILE *inputFile = fopen(argv[1], "r");		// Default place to get input

	Lexer *lex = newLexer(inputFile);
	
	Lexval val;
	Token_t typ;
	for (;;) {
		typ = getNextToken(lex, &val);
		switch (typ) {
			case END:
				printf("END\n");
				return 0;
			
			case UNKNOWN:
				printf("Unknown token");
				break;

			case NL:
				printf("NL");
				break;

			case NUMBER:
				printf("NUMBER: %f", val.value);
				break;

			case PLUS:
				printf("PLUS");
				break;

			case MINUS:
				printf("MINUS");
				break;

			case MUL:
				printf("MUL");
				break;

			case DIV:
				printf("DIV");
				break;

			case MOD:
				printf("MOD");
				break;

			case LPAREN:
				printf("LPAREN");
				break;

			case RPAREN:
				printf("RAPREN");
				break;

			default:
				printf("FAILURE");
				break;
		}
		printf("\n");
	}

	fclose(inputFile);
	freeLexer(lex);

	return 0;
}
