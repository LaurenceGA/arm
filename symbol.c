#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "headers/symbol.h"
#include "headers/lexer.h"

// static Symbol *symlist = NULL;
static Frame *frameStack = NULL, *mainframe = NULL;
//extern void refreshVars(astNode *root);

void createTable() {
	frameStack = newFrame();
	mainframe = frameStack;
}

Symbol *install(char *s, int typ, double val, bool cnst) {
	Symbol *sp = newSymbol();

	sp->name = (char *) malloc(strlen(s) + 1);
	if (sp->name == NULL) {
		fprintf(stderr, "Failed to allocate memory for new symbol name\n");
		abort();
	}

	strcpy(sp->name, s);
	sp->type = typ;
	sp->varType = NUMBER;
	sp->val = val;
	sp->cnst = cnst;

	// sp->next = symlist;
	// symlist = sp;
	sp->next = frameStack->vars;
	frameStack->vars = sp;
	return sp;
}

Symbol *newSymbol() {
	Symbol *sp = (Symbol *) malloc(sizeof(Symbol));
	if (sp == NULL) {
		fprintf(stderr, "Failed to allocate memory for new symbol\n");
		abort();
	}

	sp->name = NULL;
	sp->type = UNDEF;
	sp->varType = NUMBER;
	sp->val = 0;
	sp->cnst = false;
	sp->next = NULL;

	return sp;
}

Symbol *lookup(char *s) {
	// Check in local variables
	for (Symbol *sp = frameStack->vars; sp != NULL; sp=sp->next) {
		if (strcmp(sp->name, s) == 0) {
			return sp;
		}
	}

	// If local isn't the mainframe...
	if (frameStack == mainframe) {
		return NULL;
	} else {
		// Check global variables
		for (Symbol *sp = mainframe->vars; sp != NULL; sp=sp->next) {
			if (strcmp(sp->name, s) == 0) {
				return sp;
			}
		}
	}

	return NULL;
}

Frame *newFrame() {
	Frame *nfrm = (Frame *) malloc(sizeof(Frame));
	if (nfrm == NULL) {
		fprintf(stderr, "Failed to allocate memory for new frame\n");
		abort();
	}

	nfrm->vars = NULL;
	nfrm->next = NULL;
	nfrm->args = NULL;
	nfrm->nargs = 0;

	return nfrm;
}

Frame *loadFrame(Frame *f) {
	f->next = frameStack;
	frameStack = f;
	return frameStack;
}

Frame *copyFrame(Frame *f) {
	Frame *frm = newFrame();
	frm->nargs = f->nargs;
	frm->args = f->args;
	// Needs a fresh set of variables
	for (Symbol *s=f->vars; s != NULL; s=s->next) {
		Symbol *sym = newSymbol();
		*sym = *s;
		if (frm->vars == NULL) {
			frm->vars = sym;
		} else {
			sym->next = frm->vars;
			frm->vars = sym;
		}
	}
	// Reset connections to variables
	//loadFrame(frm);
	//refreshVars(frm->code);
	//popFrame();

	return frm;
}

Frame *popFrame() {
	Frame *top = frameStack;
	frameStack = frameStack->next;
	top->next = NULL;
	return top;
}

Frame *topFrame() {
	return frameStack;
}

void setReturn(short typ, double val, char *s) {
	frameStack->retrn.type = typ;
	if (typ == NUMBER)
		frameStack->retrn.val = val;
	else
		frameStack->retrn.str = s;
}

void dumpMainVars() {
	varDump(mainframe);
}

void varDump(Frame *f) {
	printf("\n");
	if (f == NULL) {
		printf("Empty frame.");
		return;
	}
	for (Symbol *s=f->vars; s != NULL; s = s->next) {
		if (s->type == VAR) {
			if (s->varType == NUMBER)
				printf("%s%s: %g\n", (s->cnst) ? "const ": "", s->name, s->val);
			else {
				printf("%s%s: \"%s\"\n", (s->cnst) ? "const ": "", s->name, s->str);
			}
		} else if (s->type == UNDEF) {
			printf("%s: undef\n", s->name);
		} else {
			//printf("%s\n", s->name);
		}
	}
}
