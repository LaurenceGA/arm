# arm
A rudimentary programming language with c-like syntax.
Some of the main language features are:  
* Control flow structures (if, else, while).   
* It has built in mathematical functions : sin(), cos(), sqrt(), log()... (more in math.c)
* User defined functions
* Recursion
* Comments
* Built in constants (PI, GAMMA, E, PHI)

## Examples
More examples located in the testFiles directory

A while loop and and if/else statements and some printing:
```
// Tests out breaks and continue statements as whell as while loops and printing
i=0
while i < 10 {  // Loop 10 times
	if i == 2 {
		i = i + 1
		continue
	} else if i == 5 {
		break
		print 1000000, "\n"
	}
	print i, "\n"
	i = i + 1
}
print "\n", i, "\n"
```

Print the fibbonacci sequence:
```
func fib(x) {
	if x < 2 {  // Recursive base case
		return 1
	} else {
		return fib(x-1) + fib(x-2)  // Recursive calls
	}	
}

i = 0
while i < 16 {  // Print the first 16 fib numbers
	print fib(i), " "
	i += 1
}
print // Prints a newline
```

## Operator precedence
The language support a detailde operator precedence heirachy including not only useful precedence, but also associativity. The breakdown is as follows:   

| Operator  | Description          | Associativity |
| --------- | -------------------- | ------------- |
| ( )       | Brackets             | N/A           |
| ^         | Exponentiate         | Right         |
| +-!       | Unary plus/minus/Not | N/A           |
| *%/       | Multiplicative & Mod | Left          |
| +-        | Additive				     | Left          |
| >,<,>=,<= |	Size relations			 | Left          |
| ==, !=		| Equality relational  | Left          |
| AND			  | Logical	AND				   | Left          |
| OR			  | Logical OR				   | Left          |

## Grammar
The grammar is defined as follows (from headers/parse.h)

```
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
```
