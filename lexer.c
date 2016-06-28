#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "headers/lexer.h"
#include "headers/symbol.h"

// For error messages
// Lines up with Token_t enum
char *tokens[] = {
	"unknown",
	"EOF",
	"new line",
    "number",
    "+",
    "-",
    "*",
    "/",
	"%",
	"^",
	"(",
	")",
	"variable name",
	"built in function",
	"undefined symbol",
	"=",
	"!",
	">",
	">=",
	"<",
	"<=",
	"==",
	"!=",
	"AND",
	"OR",
	"IF",
	"ELSE",
	"{",
	"}",
	"WHILE",
	"BREAK",
	"CONTINUE",
	"PRINT",
	",",
	"string",
	"PRINTLN",
	"FUNC",
	"FUNCTION",
	"RETNR"
};


Lexer *newLexer(FILE *f) {
	// Multiple lexers can be going at once
	Lexer *newLex = (Lexer *) malloc(sizeof(Lexer));
	if (newLex == NULL) {
		fprintf(stderr, "%s\n", "Could not allocate new lexer");
		abort();
	}

	newLex->inpFile = f;
	newLex->line = 1;
	newLex->buff = NULL;

	return newLex;
}

void freeLexer(Lexer *l) {
	free(l);
}

void advance(Lexer *l) {
	l->currentChar = fgetc(l->inpFile);
}

Token_t getNextToken(Lexer *l, Lexval *val) {
	// BUFFERED TOKEN
	if (l->buff != NULL) {
		*val = l->buff->lVal;	// Copy stored value to provided address
		Token_t typ = l->buff->type;
		
		TokenBuff *tok = l->buff;
		l->buff = l->buff->next;
		free(tok);

		return typ;
	}

	// NORMAL LEXING
	skipWhitespace(l);	// Space and tab
	
	advance(l);
	
	// NUMBER (unary minus is handled in parsing stage)
	if (l->currentChar == '.' || isdigit(l->currentChar)) {
		ungetc(l->currentChar, l->inpFile);
		fscanf(l->inpFile, "%lf", &val->value);
		return NUMBER;
	}

	if (isalpha(l->currentChar) || l->currentChar == '_') {
		Symbol *s;
		char sbuff[MAXVAR], *p = sbuff;

		do {
			*p++ = l->currentChar;
			advance(l);
		} while (isalnum(l->currentChar) || l->currentChar == '_');
		ungetc(l->currentChar, l->inpFile);
		*p = '\0';

		if ((s=lookup(sbuff)) == 0) {	// Not a symbol
			s = install(sbuff, UNDEF, 0, false);	// type set to var later
		}

		val->sym = s;
		return s->type == UNDEF ? VAR : s->type;
	}

	if (l->currentChar == '"') {
		char *p = val->str;
		advance(l);
		int cnt = 0;
		for (; l->currentChar != '"'; p++, cnt++) {
			if (l->currentChar == '\n' || l->currentChar == EOF) {
				fprintf(stderr, "Missing close quote\n");
				abort();	
			}
			if (cnt > MAXSTR - 1) {
				*p = '\0';
				fprintf(stderr, "String too long\n");
				abort();
			}
			*p = backslash(l, l->currentChar);
			advance(l);
		}
		*p = '\0';
		return STRING;
	}

	switch (l->currentChar) {
		case '\n':
			l->line++;
			return NL;

		case EOF:
			return END;

		case '+':
			advance(l);
			if (l->currentChar == '=') {
				val->value = (double) PLUS;
				return ASIGN;
			} else
				ungetc(l->currentChar, l->inpFile);
			return PLUS;

		case '-':
			advance(l);
			if (l->currentChar == '=') {
				val->value = (double) MINUS;
				return ASIGN;
			} else
				ungetc(l->currentChar, l->inpFile);
			return MINUS;

		case '*':
			advance(l);
			if (l->currentChar == '=') {
				val->value = (double) MUL;
				return ASIGN;
			} else
				ungetc(l->currentChar, l->inpFile);
			return MUL;

		case '/':
			// Comments
			advance(l);
			if (l->currentChar == '/') {
				while (l->currentChar != '\n')	// Throw away comment until we find the end of the line
					advance(l);
				return NL;
			} else if (l->currentChar == '=') {
				val->value = (double) DIV;
				return ASIGN;
			} else		// Otherwise it's divide
				ungetc(l->currentChar, l->inpFile);
			return DIV;

		case '%':
			advance(l);
			if (l->currentChar == '=') {
				val->value = (double) MOD;
				return ASIGN;
			} else
				ungetc(l->currentChar, l->inpFile);
			return MOD;

		case '^':
			advance(l);
			if (l->currentChar == '=') {
				val->value = (double) EXP;
				return ASIGN;
			} else
				ungetc(l->currentChar, l->inpFile);
			return EXP;

		case ';':
			return NL;

		case '(':
			return LPAREN;

		case ')':
			return RPAREN;

		case '{':
			return LBRACE;

		case '}':
			return RBRACE;

		case ',':
			return COMMA;

		case '=':
			advance(l);
			if (l->currentChar == '=')
				return EQ;
			else
				ungetc(l->currentChar, l->inpFile);
			return ASIGN;

		case '!':
			advance(l);
			if (l->currentChar == '=')
				return NEQ;
			else
				ungetc(l->currentChar, l->inpFile);
			return NOT;
		
		case '>':
			advance(l);
			if (l->currentChar == '=')
				return GE;
			else
				ungetc(l->currentChar, l->inpFile);
			return GT;

		case '<':
			advance(l);
			if (l->currentChar == '=')
				return LE;
			else
				ungetc(l->currentChar, l->inpFile);
			return LT;

		default:
			return UNKNOWN;
	}

	return UNKNOWN;
}

char backslash(Lexer *l, int c) {
	static char transtab[] = "b\bf\fn\nr\rt\t";
	if (c != '\\')
		return c;
	advance(l);
	c = l->currentChar;
	if (islower(c) && strchr(transtab, c)) {
		return strchr(transtab, c)[1];
	}
	return c;
}

void unGetToken(Lexer *l, Lexval *val, Token_t tok) {
	// Token buffering to allow for lookahead
	TokenBuff *newBuff = (TokenBuff *) malloc(sizeof(TokenBuff));
	if (newBuff == NULL) {
		fprintf(stderr, "Failed to allocate memory for token buffer\n");
		abort();
	}
	newBuff->type = tok;
	newBuff->lVal = *val;
	newBuff->next = l->buff;
	l->buff = newBuff;
}

void skipWhitespace(Lexer *l) {
	advance(l);
	while (l->currentChar == ' ' || l->currentChar == '\t')
		advance(l);
	ungetc(l->currentChar, l->inpFile);	// Went too far, don't need that
}

