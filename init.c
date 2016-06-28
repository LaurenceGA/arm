#include <stdbool.h>
#include <math.h>

#include "headers/symbol.h"
#include "headers/lexer.h"

// External math functions mapped to the builtins
extern double degtorad(), radtodeg(), Sqrt(), nlog(), Log10(), Rand();
static struct {
	char *name;
	double cval;
} consts[] = {	// Constants
	{"PI", 3.14159265358979323846},
	{"E", 2.71828182845904523536},
	{"GAMMA", 0.57721566490153286060},
	{"PHI", 1.61803398874989484820},
	{"true", 1},
	{"false", 0},
	{0, 0}		
};

static struct {
	char *name;
	char *sval;
} sconsts[] = {
	{"NL", "\n"},
	{0, 0}
};

static struct {
	char *name;
	int kval;
} keywords[] = {
	{"AND", AND},
	{"OR", OR},
	{"if", IF},
	{"else", ELSE},
	{"while", WHILE},
	{"break", BREAK},
	{"continue", CONTINUE},
	{"print", PRINT},
	{"println", PRINTLN},
	{"func", FUNC},
	{"return", RETRN},
	{0, 0}
};

static struct {
	char *name;
	double (*func)();
} builtins[] = {
	{"sin", sin},
	{"asin", asin},
	{"cos", cos},
	{"acos", acos},
	{"tan", tan},
	{"atan", atan},
	{"ln", nlog},		// > 0
	{"log", Log10},		// > 0
	{"sqrt", Sqrt},		// >= 0
	{"abs", fabs},
	{"ceil", ceil},
	{"floor", floor},
	{"degtorad", degtorad},
	{"radtodeg", radtodeg},
	{"round", round},
	{0, 0}
};

void init() {
	// Installs constants into the symbol table
	for (int i=0; consts[i].name; i++) {
		install(consts[i].name, VAR, consts[i].cval, true);
	}
	
	// Install string constants into table
	Symbol *sym;
	for (int i=0; sconsts[i].name; i++) {
		sym = install(sconsts[i].name, VAR, 0, true);
		sym->str = sconsts[i].sval;
		sym->varType = STRING;
	}

	// Install builtins into the symbol table
	Symbol *s;
	for (int i=0; builtins[i].name; i++) {
		s = install(builtins[i].name, BLTIN, 0, true);
		s->ptr = builtins[i].func;
	}

	// Install keywords into symbols
	for (int i=0; keywords[i].name; i++) {
		install(keywords[i].name, keywords[i].kval, 0, true);
	}
}

