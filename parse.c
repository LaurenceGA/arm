#include <string.h>
#include <stdlib.h>

#include "headers/parse.h"
#include "headers/lexer.h"
#include "headers/ast.h"

void init();
extern bool VARDUMP;

bool parse(FILE *f) {
	Lexer *lex = newLexer(f);
	Lexval val;		// Stores extra info about token

	createTable();	// Initializes the symbol table
	init();	// Install consts into the symbol table
	astNode *stmntList = StatementList(lex, &val, true);
	if (VARDUMP) {
		dumpMainVars();
	}
	freeTree(stmntList);
	freeLexer(lex);

	return true;
}

astNode *StatementList(Lexer *l, Lexval *val, bool exec) {
	// ((Statement)?\n)* END
	// Returns a linked list of statements
	//printf(">");	// prompt
	
	astNode *stmntList = NULL;
	astNode *tail;
	Token_t tok;
	
	while ((tok = getNextToken(l, val)) != END && tok != RBRACE && tok != ELSE) {	// Until EOF
		if (tok != NL) {				// Allows line to be emptry
			unGetToken(l, val, tok);	// Need token we requested in other functions

			astNode *statement = Statement(l, val);
			if ((tok = getNextToken(l, val)) != NL) {	// Statement ends in newline
				syntaxError("newline", tokens[tok], l->line);
			}

			// Execute as we parse if told
			if (exec)
				execute(statement);

			if (stmntList == NULL) {
				tail = stmntList = unopNode(STATEMENT_nd, NULL);
			} else {
				tail = tail->next = unopNode(STATEMENT_nd, NULL);
			}
			tail->op.left = statement;
		}
		//printf(">");
	}
	if (tok != END) {
		unGetToken(l, val, tok);
	}

	return stmntList;	// Parsed just fine
}

astNode *Statement(Lexer *l, Lexval *val) {
	// Expression|Assignment|If|While|'{'StatementList'}'
	Token_t tok = getNextToken(l, val);
	if (tok == IF) {
		unGetToken(l, val, tok);
		return If(l, val);
	} else if (tok == PRINT || tok == PRINTLN) {
		unGetToken(l, val, tok);
		return Print(l, val);
	} else if (tok == FUNC) {
		unGetToken(l, val, tok);
		return FuncDec(l, val);
	} else if (tok == RETRN) {
		unGetToken(l, val, tok);
		return Retrn(l, val);
	} else if (tok == WHILE) {
		unGetToken(l, val, tok);
		return While(l, val);
	} else if (tok == BREAK) {
		return unopNode(BREAK_nd, NULL);
	} else if (tok == CONTINUE) {
		return unopNode(CONTINUE_nd, NULL);
	} else if (tok == LBRACE) {
		astNode *lst = StatementList(l, val, false);
		if ((tok = getNextToken(l, val)) == RBRACE) {
			return lst;
		} else {
			syntaxError("right brace (})", tokens[tok], l->line);
		}
	} else if (tok == VAR) {
		Token_t typ = tok;
		Lexval varVal = *val;
		if ((tok = getNextToken(l, val)) == ASIGN) {
			unGetToken(l, val, tok);
			unGetToken(l, &varVal, typ);
			return Assignment(l, val);
		} else {
			unGetToken(l, val, tok);
			unGetToken(l, &varVal, VAR);
			return Expression(l, val);
		}
	}
	unGetToken(l, val, tok);

	return Expression(l, val);
}

astNode *Retrn(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	if (tok == RETRN) {
		return unopNode(RETRN_nd, Expression(l, val));
	}
	syntaxError("return statement", tokens[tok], l->line);
	return NULL;
}

astNode *FuncDec(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	if (tok == FUNC) {
		if ((tok = getNextToken(l, val)) == VAR) {
			Lexval varVal = *val;
			if ((tok = getNextToken(l, val)) == LPAREN) {
				// Load template frame
				varVal.sym->templt = newFrame();
				varVal.sym->type = FUNCTION;
				loadFrame(varVal.sym->templt);

				// Arguments
				astNode *args = NULL;
				astNode *curArg = NULL;
				if ((tok = getNextToken(l, val)) == VAR) {
					args = curArg = opNode(ARG_nd, varNode(val), NULL);
					
					while ((tok = getNextToken(l, val)) == COMMA) {
						if ((tok = getNextToken(l, val)) == VAR) {
							curArg->next = opNode(ARG_nd, varNode(val), NULL);
							curArg = curArg->next;
						} else {
							syntaxError("argument name", tokens[tok], l->line);
						}
					}
					unGetToken(l, val, tok);
				} else unGetToken(l, val, tok);

				// End of arguments
				if ((tok = getNextToken(l, val)) == RPAREN) {
					astNode *funcnode = opNode(FUNCDEC_nd, varNode(&varVal), NULL);
					
					funcnode->next = args;
					
					if ((tok = getNextToken(l, val)) != NL) {
						unGetToken(l, val, tok);
					}
					
					funcnode->op.right = Statement(l, val);
					
					// Unload template frame
					popFrame();

					return funcnode;
				} else syntaxError("right parentheses", tokens[tok], l->line);
			} else syntaxError("left parentheses", tokens[tok], l->line);
		}
	}
	syntaxError("function declaration", tokens[tok], l->line);
	return NULL;
}

astNode *Print(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	if (tok == PRINT || tok == PRINTLN) {
		Token_t prType = tok;
		if ((tok = getNextToken(l, val)) != NL) {
			unGetToken(l, val, tok);
			astNode *curArg = NULL;	// For multiple args
			astNode *prNode = unopNode(PRINT_nd, (curArg = opNode(ARG_nd, Expression(l, val), NULL)));

			while ((tok = getNextToken(l, val)) == COMMA) {
				curArg->next = opNode(ARG_nd, Expression(l, val), NULL);
				curArg = curArg->next;
			}

			unGetToken(l, val, tok);

			Lexval nl;
			nl.sym = lookup("NL");
			if (prType == PRINTLN) {
				curArg->next = opNode(ARG_nd, varNode(&nl), NULL);
			}

			return prNode;
		} else {
			unGetToken(l, val, tok);
			return unopNode(PRINT_nd, NULL);
		}
	}
	return NULL;
}

astNode *If(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	if (tok == IF) {
		astNode *ifnode = unopNode(IF_nd, Expression(l, val));
		if ((tok = getNextToken(l, val)) != NL) {
			unGetToken(l, val, tok);
		}

		ifnode->op.left = Statement(l, val);
		
		// Optional else statement
		if ((tok = getNextToken(l, val)) == ELSE) {
			ifnode->op.right = Statement(l, val);
		} else {
			unGetToken(l, val, tok);
		}
		
		return ifnode;
	}
	syntaxError("if statement", tokens[tok], l->line);
	return NULL;
}

astNode *While(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	if (tok == WHILE) {
		astNode *condition = Expression(l, val);
		if ((tok = getNextToken(l, val)) != NL) {
			unGetToken(l, val, tok);
		}

		return opNode(WHILE_nd, condition, Statement(l, val));
	}
	syntaxError("while statement", tokens[tok], l->line);
	return NULL;
}

astNode *Assignment(Lexer *l, Lexval *val) {
	Token_t tok = getNextToken(l, val);
	Lexval varVal = *val;

	if (tok == VAR) {
		if ((tok = getNextToken(l, val)) == ASIGN) {
			astNode *next;
			Token_t augType = (Token_t) val->value;
			astNode *asgnNode = opNode(ASGN_nd, varNode(&varVal), Expression(l, val));

			switch (augType) {
				case PLUS:
					next = unopNode(ADD_nd, NULL);
					break;
				case MINUS:
					next = unopNode(SUB_nd, NULL);
					break;
				case MUL:
					next = unopNode(MUL_nd, NULL);
					break;
				case DIV:
					next = unopNode(DIV_nd, NULL);
					break;
				case MOD:
					next = unopNode(MOD_nd, NULL);
					break;
				case EXP:
					next = unopNode(EXP_nd, NULL);
					break;

				default:
					next = NULL;
					break;
			}

			asgnNode->next = next;

			return asgnNode;
		}
	}
	return NULL;
}

astNode *Expression(Lexer *l, Lexval *val) {
	// LogicOR('AND'LogicOR)*
	astNode *node = LogicOR(l, val);

	Token_t tok;
	// Check if next token is a operator of this precedence
	while ((tok = getNextToken(l, val)) == OR) {
		node = opNode(OR_nd, node, LogicOR(l, val));
	}

	unGetToken(l, val, tok);	// If it wasn't, give it back

	return node;
}

astNode *LogicOR(Lexer *l, Lexval *val) {
	// LogicAND('AND'LogicAND)*
	astNode *node = LogicAND(l, val);

	Token_t tok;
	// Check if next token is a operator of this precedence
	while ((tok = getNextToken(l, val)) == AND) {
		node = opNode(AND_nd, node, LogicAND(l, val));
	}

	unGetToken(l, val, tok);	// If it wasn't, give it back

	return node;
}

astNode *LogicAND(Lexer *l, Lexval *val) {
	// EqRelOp(('=='|'!=')EqRelOp)*
	astNode *node = EqRelOp(l, val);

	Token_t tok;
	// Check if next token is a operator of this precedence
	while ((tok = getNextToken(l, val)) == EQ || tok == NEQ) {
		if (tok == EQ) {
			node = opNode(EQ_nd, node, EqRelOp(l, val));
		} else if (tok == NEQ) {
			node = opNode(NEQ_nd, node, EqRelOp(l, val));
		}
	}

	unGetToken(l, val, tok);	// If it wasn't, give it back

	return node;
}

astNode *EqRelOp(Lexer *l, Lexval *val) {
	// SzRelOp(('>'|'<'|'>='|'<=')SzRelOp)*
	astNode *node = SzRelOp(l, val);

	Token_t tok;
	// Check if next token is a operator of this precedence
	while ((tok = getNextToken(l, val)) == GT || tok == LT || tok == GE || tok == LE) {
		if (tok == GT) {
			node = opNode(GT_nd, node, SzRelOp(l, val));
		} else if (tok == LT) {
			node = opNode(LT_nd, node, SzRelOp(l, val));
		} else if (tok == GE) {
			node = opNode(GE_nd, node, SzRelOp(l, val));
		} else if (tok == LE) {
			node = opNode(LE_nd, node, SzRelOp(l, val));
		}
	}

	unGetToken(l, val, tok);	// If it wasn't, give it back

	return node;

}

astNode *SzRelOp(Lexer *l, Lexval *val) {
	// Term(('+'|'-')Term)*
	astNode *node = Term(l, val);

	Token_t tok;
	// Check if next token is a operator of +- precedence
	while ((tok = getNextToken(l, val)) == PLUS || tok == MINUS) {
		if (tok == PLUS) {
			node = opNode(ADD_nd, node, Term(l, val));
		} else {
			node = opNode(SUB_nd, node, Term(l, val));
		}
	}

	unGetToken(l, val, tok);	// If it wasn't, give it back

	return node;
}

astNode *Term(Lexer *l, Lexval *val) {
	// Factor(('*'|'/'|'%')Factor)*
	astNode *node = Factor(l, val);

	Token_t tok;
	// Check if of */% precedence
	while ((tok = getNextToken(l, val)) == MUL || tok == DIV || tok == MOD) {
		if (tok == MUL) {
			node = opNode(MUL_nd, node, Factor(l, val));
		} else if (tok == DIV) {
			node = opNode(DIV_nd, node, Factor(l, val));
		} else if (tok == MOD) {
			node = opNode(MOD_nd, node, Factor(l, val));
		} else {
			// Impossible
		}
	}

	unGetToken(l, val, tok);	// If not, give back

	return node;
}

astNode *Factor(Lexer *l, Lexval *val) {
	// (['!']['+'|'-']Factor)|Expo
	Token_t tok;
	if ((tok = getNextToken(l, val)) == PLUS || tok == MINUS || tok == NOT) {
		if (tok == MINUS) {
			return unopNode(NEG_nd, Factor(l, val));
		} else if (tok == PLUS) {
			return Factor(l, val);
		} else if (tok == NOT) {
			return unopNode(NOT_nd, Factor(l, val));
		} else {
			unGetToken(l, val, tok);
			return Expo(l, val);
		}
	}

	unGetToken(l, val, tok);

	return Expo(l, val);
}

astNode *Expo(Lexer *l, Lexval *val) {
	// Final(^Final)*
	astNode *node = Final(l, val);

	Token_t tok;
	while ((tok = getNextToken(l, val)) == EXP) {
		// EXP is right associative
		if (node->type != EXP_nd) {
			node = opNode(EXP_nd, node, Final(l, val));
		} else {
			// Final right-most exp op
			astNode *rightOp = node;
			while (rightOp->op.right->type != NUM_nd) {
				rightOp = rightOp->op.right;
			}
			rightOp->op.right = opNode(EXP_nd, rightOp->op.right, Final(l, val));
		}
	}

	unGetToken(l, val, tok);	// Don't need it

	return node;
}

astNode *Final(Lexer *l, Lexval *val) {
	// BLTIN'('Expression')'|NUM|'('Expression')'|VAR|FUNCTION'('Expression...')'|STRING
	Token_t tok = getNextToken(l, val);
	if (tok == NUMBER) {
		return numNode(val->value);
	} else if (tok == STRING) {
		return strNode(val);
	} else if (tok == VAR) {
		return varNode(val);
	} else if (tok == FUNCTION) {
		Symbol *s = val->sym;
		if ((tok = getNextToken(l, val)) == LPAREN) {
			// Arguments
			astNode *args = NULL;
			astNode *curArg = NULL;
			if ((tok = getNextToken(l, val)) != RPAREN) {
				unGetToken(l, val, tok);
				args = curArg = opNode(ARG_nd, Expression(l, val), NULL);
				
				while ((tok = getNextToken(l, val)) == COMMA) {
					curArg->next = opNode(ARG_nd, Expression(l, val), NULL);
					curArg = curArg->next;
				}
				unGetToken(l, val, tok);
			} else unGetToken(l, val, tok);
			// End of arguments

			if ((tok = getNextToken(l, val)) == RPAREN) {
				astNode *fncNode = unopNode(FNC_nd, args);
				fncNode->sym = s;
				return fncNode;
			} else {
				syntaxError(")", tokens[tok], l->line);
			}
		} else {
			syntaxError("(", tokens[tok], l->line);
		}
	} else if (tok == BLTIN) {
		Symbol *s = val->sym;
		if ((tok = getNextToken(l, val)) == LPAREN) {
			astNode *arg = Expression(l, val);
			if ((tok = getNextToken(l, val)) == RPAREN) {
				astNode *bltNode = unopNode(BLTIN_nd, arg);
				bltNode->sym = s;
				return bltNode;
			} else {
				syntaxError(")", tokens[tok], l->line);
			}
		} else {
			syntaxError("(", tokens[tok], l->line);
		}
	} else if (tok == LPAREN) {							// Parse '('
		astNode *node = Expression(l, val);				// Prase Expression
		if ((tok = getNextToken(l, val)) != RPAREN) {	// Parse ')'
			syntaxError(")", tokens[tok], l->line);
		}
		return node;
	}

	syntaxError("Expression", tokens[tok], l->line);
	return NULL;
}

void syntaxError(char *wanted, char *got, int line) {
	fprintf(stderr, "Syntax error (%d): wanted '%s', got '%s'\n", line, wanted, got);
	abort();
}
