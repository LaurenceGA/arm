#ifndef AST_H
#define AST_H

//#include "symbol.h"
#include "lexer.h"

// Our types
typedef enum nodeType nodeType;
typedef struct astNode astNode;

// AST NODE
enum nodeType {
	STATEMENT_nd,

	NUM_nd,
	STR_nd,
	VAR_nd,
	BLTIN_nd,

	ADD_nd,		// +
	SUB_nd,		// -
	MUL_nd,		// *
	DIV_nd,		// /
	EXP_nd,		// ^
	MOD_nd,		// %

	NEG_nd,		// -
	NOT_nd,		// !

	ASGN_nd,	// =

	GT_nd,		// >
	LT_nd,		// <
	GE_nd,		// >=
	LE_nd,		// <=

	EQ_nd,		// ==
	NEQ_nd,		// !=

	AND_nd,		// AND
	OR_nd,		// OR

	IF_nd,
	WHILE_nd,

	BREAK_nd,
	CONTINUE_nd,

	PRINT_nd,
	ARG_nd,		// Linked list, holds arg in it's left branch

	FUNCDEC_nd,
	FNC_nd,
	RETRN_nd
};

struct astNode {
	nodeType type;
	double value;	// For number nodes
	char *str;		// For str nodes
	astNode *next;
	Symbol *sym;
	struct {
		astNode *left;
		astNode *right;
	} op;
};

double getValue(astNode *root);		// Returns the value of a tree - called on operators

astNode *newNode(nodeType typ);
astNode *opNode(nodeType typ, astNode *left, astNode *right);	// Binary operator +-^%* etc.
astNode *unopNode(nodeType typ, astNode *next);	// Unary operators
astNode *numNode(double value);	// Just numbers
astNode *varNode(Lexval *lval);
astNode *strNode(Lexval *lval);

Frame *callFunc(astNode *root);
void refreshVars(astNode *root);

void execute(astNode *root);
void freeTree(astNode *root);

void printTree(astNode *root, int level, bool nxt);	// Recursive print

#endif

