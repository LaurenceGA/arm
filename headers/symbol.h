#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdbool.h>

// Need to know about ast nodes
typedef struct astNode astNode;

typedef struct Symbol {
	char *name;
	short type;				// VAR, UNDEF, BLTIN etc.
	short varType;			// Only used if type is VAR
	bool cnst;
	union {
		double val;			// VAR
		double (*ptr)();	// BLTIN
		char *str;			// STRING
		struct {
			struct Frame *templt;
			astNode *code;
		};
	};
	struct Symbol *next;
} Symbol;

Symbol *newSymbol();
Symbol *install(char *s, int typ, double val, bool cnst);
Symbol *lookup(char *s);

typedef struct Frame {
	Symbol *vars;	// Variables local to the frame
	astNode *args;	// Linked list of args
	int nargs;
	struct Frame *next;
	struct {
		short type;	// STRING or NUMBER
		union {
			double val;
			char *str;
		};
	} retrn;
} Frame;

void createTable();
Frame *newFrame();

Frame *loadFrame(Frame *f);
Frame *copyFrame(Frame *f);
Frame *popFrame();
Frame *topFrame();

void setReturn(short typ, double val, char *s);

void dumpMainVars();
void varDump(Frame *f);

#endif
