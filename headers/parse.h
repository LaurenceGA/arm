#ifndef PARSE_H
#define PARSE_H

#include <stdio.h>
#include <stdbool.h>

#include "lexer.h"
#include "ast.h"

/*
Prases the grammar:

StatementList:
	((Statement)?\n)* END

Statement:	
	Expression|Assignment|If|While|
	'{'StatementList'}'|BREAK|
	CONTINUE|Print|FuncDec|Retrn

Retrn:
	RETURN Expression

FuncDec:
	'func' VAR'('(VAR(,VAR)*)?')'\n? Statement

If:
	IF Expression\n? Statement (ELSE Statement)?

Print:
	PRINT (Expression(,Expression)*)?

While:
	WHILE Expression\n? Statement

Assignment:
	VAR '=' Expression

Expression:
	LogicOR('OR'LogicOR)*

LogicOR:
	LogicAND('AND'LogicAND)*

LogicAND:
	EqRelOp(('=='|'!=')EqRelOp)*

EqRelOp:
	SzRelOp(('>'|'<'|'>='|'<=')SzRelOp)*

SzRelOp:
	Term(('+'|'-')Term)*

Term:
	Factor(('*'|'/'|'%')Factor)*

Factor:
	['!']['-'|'+']Expo

Expo:
	Final('^'Final)*

Final:
	BLTIN'('Expression')'|VAR|NUM|'('Expression')'|STRING|FUNC'('Expression{NARGS}')'

This produces the following operator precedence
Operator	Description				Associativity
()			Brackets				N/A
^			Exponentiate			Right
+-!			Unary plus/minus/Not	N/A
*%/			Multiplicative & Mod	Left
+-			Additive				Left
>,<,>=,<=	Size relations			Left
==, !=		Equality relational		Left
AND			Logical	AND				Left
OR			Logical OR				Left

*/

bool parse(FILE *f);

astNode *StatementList(Lexer *l, Lexval *val, bool exec);

// All of the following take (Lexer *l, Lexval *val) arguments
astNode *Statement(), *Expression(), *LogicOR(), *Print(),
		*LogicAND(), *EqRelOp(), *SzRelOp(), *Assignment(),
		*Term(), *Factor(), *Expo(), *Final(), *If(), *While(),
		*FuncDec(), *Retrn();

void syntaxError(char *wanted, char *got, int line);

#endif
