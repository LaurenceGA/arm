#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "headers/parse.h"

/* arm is a simple interpreted language */

bool SHOWTREE = false;
bool VARDUMP = false;

void printUsage(char *progName) {
	fprintf(stderr, "usage: %s [-tv] [inputFile]\n", progName);
}

int main(int argc, char *argv[]) {
	FILE *inputFile = NULL;		// Default place to get input
	bool fromInp = false;

	// -Options
	// -t prints syntax tree after each statement
	int fileArg = 1;

	if (argc > 1 && argv[1][0] == '-') {
		// Can't just have '-' as an argument
		if (argv[1][1] == '\0') {
			fprintf(stderr, "Option must be specified\n");
			printUsage(argv[0]);
			abort();
		}

		for (int i=1; argv[1][i] != '\0'; i++) {
			switch (argv[1][i]) {
				case 't':
					SHOWTREE = true;
					break;

				case 'v':
					VARDUMP = true;
					break;

				default:
					fprintf(stderr, "Unknown option '%c'\n", argv[1][i]);
					printUsage(argv[0]);
					abort();
			}
		}
		argc--; fileArg = 2;		// So we can continue as normal
	}

	if (argc == 1) {
		inputFile = stdin;
		fromInp = true;
	} else if (argc == 2) {
		inputFile = fopen(argv[fileArg], "r");
		if (!inputFile) {
			fprintf(stderr, "Failed to open '%s'\n", argv[fileArg]);
			return 1;
		}
		printf("Taking input from file '%s'\n", argv[fileArg]);
	} else {
		printUsage(argv[0]);
		return 1;
	}

	if (!parse(inputFile)) {
		fprintf(stderr, "!Failed to parse!\n");
	}

	if (!fromInp) {
		fclose(inputFile);
	}

	return 0;
}
