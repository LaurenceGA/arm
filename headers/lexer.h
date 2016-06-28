#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

#include "symbol.h"

#define MAXSTR 100
#define MAXVAR 100

/* Lexer tokenizes an input stream, passed as an open file 
 * Does not handle the openeing or closing of files */

// All the tokens types that can be parsed
typedef enum Token_t Token_t;
enum Token_t {
	UNKNOWN,
	END,
	NL,			// \n
	NUMBER,		// 1.2, .5, 3
	PLUS,		// +
	MINUS,		// -
	MUL,		// *
	DIV,		// /
	MOD,		// %
	EXP,		// ^
	LPAREN,		// (
	RPAREN,		// )
	VAR,		// PI, E, custom...
	BLTIN,		// Built in functions sin, cos
	UNDEF,		// New variable - undefined yet
	ASIGN,		// =
	NOT,		// !
	GT,			// >
	GE,			// >=
	LT,			// <
	LE,			// <=
	EQ,			// ==
	NEQ,		// !=
	AND,		// AND
	OR,			// OR
	IF,			// if
	ELSE,		// else
	LBRACE,		// {
	RBRACE,		// }
	WHILE,		// while
	BREAK,		// break
	CONTINUE,	// continue
	PRINT,		// print
	COMMA,		// ,
	STRING,
	PRINTLN,
	FUNC,		// func keyword
	FUNCTION,	// function name
	RETRN
};

// For error message purposes
extern char *tokens[];

// Stores the value information about the token to go with the type
typedef union Lexval Lexval;
union Lexval {
	double value;
	Symbol *sym;
	char str[MAXSTR];
};

typedef struct TokenBuff {
	Token_t type;
	Lexval lVal;
	struct TokenBuff *next;
} TokenBuff;

// Holds info about the lexer for a given stream
typedef struct Lexer Lexer;
struct Lexer {
	FILE *inpFile;
	char currentChar;
	int line;

	TokenBuff *buff;
};

// Public
Lexer *newLexer(FILE *f);	// Allocate memory and initialize
void freeLexer(Lexer *l);	// Free memory
Token_t getNextToken(Lexer *l, Lexval *val);	// Returns what type and sets value if appropriate;
void unGetToken(Lexer *l, Lexval *val, Token_t tok);
char backslash(Lexer *l, int c);

// Private
void advance(Lexer *l);		// Move the lexer forward a character
void skipWhitespace(Lexer *l);	// Spaces and tabs are skipped


#endif
