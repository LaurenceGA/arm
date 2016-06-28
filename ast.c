#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "headers/ast.h"
#include "headers/lexer.h"
#include "headers/symbol.h"

extern bool SHOWTREE;

static bool brk = false;
static bool cont = false;
static bool retrn = false;

double getValue(astNode *root) {
	switch (root->type) {
		case NUM_nd:
			return root->value;
			break;
		
		case ADD_nd:
			return getValue(root->op.left) + getValue(root->op.right);
			break;

		case SUB_nd:
			return getValue(root->op.left) - getValue(root->op.right);
			break;

		case MUL_nd:
			return getValue(root->op.left) * getValue(root->op.right);
			break;

		case DIV_nd:
			{
				double rval = getValue(root->op.right);
				if (rval == 0) {
					fprintf(stderr, "Divison by 0.\n");
					abort();
				}
				return getValue(root->op.left) / rval;
			}
			break;

		case EXP_nd:
			return pow(getValue(root->op.left), getValue(root->op.right));
			break;
		
		case MOD_nd:
			return (int) getValue(root->op.left) % (int) getValue(root->op.right);
			break;

		case NEG_nd:
			return -getValue(root->next);
			break;

		case NOT_nd:
			return !getValue(root->next);

		case GT_nd:
			return (double) getValue(root->op.left) > getValue(root->op.right);
		
		case LT_nd:
			return (double) getValue(root->op.left) < getValue(root->op.right);

		case GE_nd:
			return (double) getValue(root->op.left) >= getValue(root->op.right);

		case LE_nd:
			return (double) getValue(root->op.left) <= getValue(root->op.right);

		case EQ_nd:
			return (double) getValue(root->op.left) == getValue(root->op.right);

		case NEQ_nd:
			return (double) getValue(root->op.left) != getValue(root->op.right);

		case AND_nd:
			if (!getValue(root->op.left))
				return 0;
			if (!getValue(root->op.right))
				return 0;
			return 1;

		case OR_nd:
			if (getValue(root->op.left))
				return 1;
			if (getValue(root->op.right))
				return 1;
			return 0;

		case VAR_nd:
			if (root->sym->type == UNDEF) {
				fprintf(stderr, "Variable '%s' is undefined.\n", root->sym->name);
				abort();
			}
			return root->sym->val;
		
		case BLTIN_nd:
			return root->sym->ptr(getValue(root->next));

		case FNC_nd:
			{
				Frame *frm = callFunc(root);
				if (retrn) {
					retrn  = false;
					if (frm->retrn.type == NUMBER) {
						return frm->retrn.val;
					} else {
						fprintf(stderr, "Function returns string, can't get value\n");
						abort();
					}
				} else {
					fprintf(stderr, "Nothing returned!\n");
					abort();
				}
				return 0;
			}

		default:
			fprintf(stderr, "Unable to get ast node value\n");
			abort();
	}
	return 0;
}

astNode *newNode(nodeType typ) {
	astNode *node = (astNode *) malloc(sizeof(astNode));
	if (node == NULL) {
		fprintf(stderr, "Failed to allocate memory creating astNode\n");
		abort();
	}

	node->type = typ;
	node->value = 0;
	node->str = NULL;
	node->op.left = node->op.right = node->next = NULL;

	return node;
}

astNode *opNode(nodeType typ, astNode *left, astNode *right) {
	astNode *opNd = newNode(typ);

	opNd->op.left = left;
	opNd->op.right = right;

	return opNd;
}

astNode *unopNode(nodeType typ, astNode *next) {
	astNode *unopNd = newNode(typ);

	unopNd->next = next;

	return unopNd;
}

astNode *numNode(double value) {
	astNode *numNd = newNode(NUM_nd);

	numNd->value = value;

	return numNd;
}

astNode *varNode(Lexval *lval) {
	astNode *varNd = newNode(VAR_nd);

	varNd->sym = lval->sym;

	return varNd;
}

astNode *strNode(Lexval *lval) {
	astNode *strNd = newNode(STR_nd);

	strNd->str = (char *) malloc(strlen(lval->str) + 1);
	strcpy(strNd->str, lval->str);

	return strNd;
}

void execute(astNode *root) {
	if (SHOWTREE) {		// Set by cmd line option
		printTree(root, 0, false);
	}
	
	if (root == NULL) return;
	if (root->type == ASGN_nd) {
		// The symbol we're assigning to
		Symbol *s = root->op.left->sym;
		if (s->cnst) {
			fprintf(stderr, "Cannot assign to constant value '%s'\n", s->name);
			abort();
		}

		if (root->op.right->type == STR_nd) {
			if (s->varType == STRING)
				free(s->str);
			s->type = VAR;
			s->varType = STRING;
			s->str = (char *) malloc(strlen(root->op.right->str)+1);
			if (s->str == NULL) {
				fprintf(stderr, "Str memory allocation failed\n");
				abort();
			}
			strcpy(s->str, root->op.right->str);
			// s->str = root->op.right->str;
		} else if (root->op.right->type == VAR_nd &&
				root->op.right->sym->varType == STRING) {
		   if (s->varType == STRING)
			   free(s->str);
		   s->type = VAR;
		   s->varType = STRING;

		   char *rstr = root->op.right->sym->str;
		   s->str = (char *) malloc(strlen(rstr)+1);
		   if (s->str == NULL) {
				fprintf(stderr, "Str memory allocation failed\n");
				abort();
		   }
		   strcpy(s->str, rstr);
		   // s->str = root->op.right->sym->str;
		} else {
			s->type = VAR;
			s->varType = NUMBER;
			if (root->next != NULL) {
				switch (root->next->type) {
					case ADD_nd:
						s->val += getValue(root->op.right);
						break;
					case SUB_nd:
						s->val -= getValue(root->op.right);
						break;
					case MUL_nd:
						s->val *= getValue(root->op.right);
						break;
					case DIV_nd:
						s->val /= getValue(root->op.right);
						break;
					case MOD_nd:
						s->val = (int) s->val % (int) getValue(root->op.right);
						break;
					case EXP_nd:
						s->val = pow(s->val, getValue(root->op.right));
						break;

					default:
						// Shouldn't happen
						break;
				}
			} else {
				s->val = getValue(root->op.right);
			}
		}
	} else if (root->type == FUNCDEC_nd) {
		Symbol *s = root->op.left->sym;
		if (s->cnst) {
			fprintf(stderr, "Cannot create function with constant variable name '%s'\n", s->name);
			abort();
		}

		s->type = FUNCTION;
		s->code = root->op.right;
		s->templt->args = root->next;
		for (astNode *n=root->next; n != NULL; n=n->next) {
			s->templt->nargs++;
		}
		// printf("%d arguments\n", s->templt->nargs);
	} else if (root->type == FNC_nd) {
		callFunc(root);
	} else if (root->type == IF_nd) {
		if (getValue(root->next) != 0) {
			execute(root->op.left);
		} else if (root->op.right != NULL) {
			execute(root->op.right);
		}
	} else if (root->type == WHILE_nd) {
		while (getValue(root->op.left) != 0) {
			execute(root->op.right);
			if (cont) {
				cont = false;
				continue;
			}
			if (brk) {
				brk = false;
				break;
			}
		}
	} else if (root->type == STATEMENT_nd) {
		if (root->op.left != NULL) {
			execute(root->op.left);
		}
		if (!cont && !brk && !retrn) {
			if (root->next != NULL) {
				execute(root->next);
			}
		}
	} else if (root->type == BREAK_nd) {
		brk = true;
	} else if (root->type == CONTINUE_nd) {
		cont = true;
	} else if (root->type == RETRN_nd) {
		if (root->next->type == STR_nd) {
			setReturn(STRING, 0, root->next->str);
		} else {
			setReturn(NUMBER, getValue(root->next), NULL);
		}
		retrn = true;
	} else if (root->type == PRINT_nd) {
		if (root->next != NULL) {
			for (astNode *p = root->next; p != NULL; p = p->next) {
				if (p->op.left != NULL) {
					if (p->op.left->type == STR_nd) {
						printf("%s", p->op.left->str);
					} else {
						if (p->op.left->type == VAR_nd) {
							if (p->op.left->sym->varType == NUMBER) {
								printf("%g", getValue(p->op.left));
							} else {
								printf("%s", p->op.left->sym->str);
							}
						} else if (p->op.left->type == FNC_nd) {
							Frame *frm = callFunc(root);
							if (retrn) {
								retrn  = false;
								if (frm->retrn.type == NUMBER) {
									printf("%g", frm->retrn.val);
								} else {
									printf("%s", frm->retrn.str);
								}
							} else {
								fprintf(stderr, "Nothing returned!\n");
								abort();
							}
						} else
							printf("%g", getValue(p->op.left));
					}
				} else {
					fprintf(stderr, "Empty argument node\n");
					abort();
				}
			}
		} else printf("\n");
	} else if (root->type != STR_nd) {
		getValue(root);
	}
}

Frame *callFunc(astNode *root) {
	Frame *f = loadFrame(copyFrame(root->sym->templt));
	//loadFrame(root->sym->templt);
	//refreshVars(root->sym->code);
	refreshVars(f->args);

	// Load up arguments
	int supplied = 0;
	astNode *argVars = f->args;
	for (astNode *arg=root->next; arg != NULL; arg=arg->next) {
		supplied++;
		if (argVars == NULL) {
			fprintf(stderr, "Too many arguments supplied\nOnly wanted %d\n",
					f->nargs);
			abort();
		}

		if (arg->op.left->type == STR_nd) {
			Symbol *s = argVars->op.left->sym;
			if (s->varType == STRING)
				free(s->str);
			s->type = VAR;
			s->varType = STRING;
			s->str = arg->op.left->str;
		} else {
			Symbol *s = argVars->op.left->sym;
			s->type = VAR;
			s->varType = NUMBER;
			s->val = getValue(arg->op.left);
		}
		argVars = argVars->next;
	}
	if (argVars != NULL) {
		fprintf(stderr, "Not enough arguments supplied\nWanted %d, got %d\n", 
				f->nargs, supplied);
		abort();
	}
	refreshVars(root->sym->code);

	execute(root->sym->code);
	f = popFrame();
	Frame *top = topFrame();
	if (top->next != NULL) {
		refreshVars(root->sym->code);
		refreshVars(top->args);
	}
	return f;
}

void refreshVars(astNode *root) {
	if (root != NULL) {
		if (root->type == VAR_nd) {
			Symbol *s = lookup(root->sym->name);
			if (s == NULL) {
				fprintf(stderr, "Couldn't find %s\n", root->sym->name);
				abort();
			}
			root->sym = s;
		}
		refreshVars(root->next);
		refreshVars(root->op.left);
		refreshVars(root->op.right);
	}
}

void printTree(astNode *root, int level, bool nxt) {
	if (root != NULL) {
		// LEFT
		if (root->op.left != NULL) {
			printTree(root->op.left, level + 1, false);
		}
		
		// ROOT
		if (nxt) {
			printf("───┼");
		} else {
			for (int i=0; i < level; i++) {
				if (i == level-1)
					printf("───┼");
				else
					printf("   ├");
			}
		}

		switch (root->type) {
			case NUM_nd:
				printf("%g\n", root->value);
				break;
			case ADD_nd:
				printf("ADD\n");
				break;
			case SUB_nd:
				printf("SUB\n");
				break;
			case MUL_nd:
				printf("MUL\n");
				break;
			case DIV_nd:
				printf("DIV\n");
				break;
			case EXP_nd:
				printf("EXPO\n");
				break;
			case MOD_nd:
				printf("MOD\n");
				break;
			case NEG_nd:
				printf("NEG");
				break;
			case ASGN_nd:
				printf("ASGN\n");
				break;
			case VAR_nd:
				printf("%s\n", root->sym->name);
				break;
			case BLTIN_nd:
				printf("%s", root->sym->name);
				break;
			case NOT_nd:
				printf("NOT");
				break;
			case GT_nd:
				printf("GT\n");
				break;
			case LT_nd:
				printf("LT\n");
				break;
			case GE_nd:
				printf("GE\n");
				break;
			case LE_nd:
				printf("LE\n");
				break;
			case EQ_nd:
				printf("EQ\n");
				break;
			case NEQ_nd:
				printf("NEQ\n");
				break;
			case AND_nd:
				printf("AND\n");
				break;
			case OR_nd:
				printf("OR\n");
				break;
			case IF_nd:
				printf("IF");
				break;
			case WHILE_nd:
				printf("WHILE");
				break;
			case STATEMENT_nd:
				printf("STMNT");
				break;
			case BREAK_nd:
				printf("BREAK");
				break;
			case CONTINUE_nd:
				printf("CONTINUE");
				break;
			case PRINT_nd:
				printf("PRINT");
				break;
			case ARG_nd:
				printf("ARG");
				break;
			case FUNCDEC_nd:
				printf("func decl");
				break;
			case FNC_nd:
				printf("%s", root->sym->name);
				break;
			case RETRN_nd:
				printf("RETURN");
				break;
			default:
				printf("Unknown");
				break;
		}
	}

	// Next
	if (root->next != NULL) {
		printTree(root->next, level, true);
	}

	// RIGHT
	if (root->op.right != NULL) {
		printTree(root->op.right, level + 1, false);
	}
}

void freeTree(astNode *root) {
	if (root != NULL) {
		if (root->next != NULL) freeTree(root->next);
		if (root->op.left != NULL) freeTree(root->op.left);
		if (root->op.right != NULL) freeTree(root->op.right);
		if (root->str != NULL) free(root->str);
		free(root);
	}
}
