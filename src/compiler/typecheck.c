#include "includes/include.h"

// Function Prototypes
char typeCheckFunc(Func fn);
char typeCheckInstr(Instr instr);
char typeCheckPrint(Print print, char* fname, int lineNo, char* orig);
char typeCheckAssign(Assign assign, char* fname, int lineNo, char* orig);
char typeCheckFor(For fore, char* fname, int lineNo, char* orig);
char typeCheckWhile(While whilee, char* fname, int lineNo, char* orig);
char typeCheckIf(If ifi, char* fname, int lineNo, char* orig);
char typeCheckReturn(Return returni, char* fname, int lineNo, char* orig);

char verifyList(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyInt(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyBool(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyNum(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyStr(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyObj(Expr expr, char* fname, int lineNo, char* orig, char printIt);
char verifyVar(Expr expr, char* fname, int lineNo, char* orig, char printIt);

char checkExpr(Expr expr, char* fname, int lineNo, char* orig, char printIt);

// Type check a whole program
char typeCheck(AST* ast)
{
	// Loop throug the functions, and check them all
	for (int i = 0; i < ast->funcc; i++)
		if (!typeCheckFunc(ast->functions[i]))
			return 0;
	return 1;
}

// Type check a function
char typeCheckFunc(Func fn)
{
	// Loop through instructions and check them all
	for (int i = 0; i < fn.icount; i++)
		if (!typeCheckInstr(fn.instrs[i]))
			return 0;
	return 1;
}

// Type check instruction
char typeCheckInstr(Instr instr)
{
	// Extract debug information
	char* fname = instr.fname;
	int lineNo = instr.lineNo;
	char* orig = instr.orig;

	// Check according to the type of function
	switch (instr.type)
	{
		case PRINT:
			return typeCheckPrint(*(instr.data.printi), fname, lineNo, orig);
		case FOR:
			return typeCheckFor(*(instr.data.fori), fname, lineNo, orig);
		case WHILE:
			return typeCheckWhile(*(instr.data.whilei), fname, lineNo, orig);
		case ASSIGN:
			return typeCheckAssign(*(instr.data.assigni), fname, lineNo, orig);
		case IF:
			return typeCheckIf(*(instr.data.ifi), fname, lineNo, orig);
		case CONTROL:
			return 1; // Literally cannot fail
		case RETURN:
			return typeCheckReturn(*(instr.data.returni), fname, lineNo, orig);
		default:
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
					KRESET":\n\t" KCYN "%s" KRESET "\n\tUnrecognised Instruction.\n",
					fname, lineNo, orig);
			return 0;
	}
	return 1;
}

// Print statements - just check the expression's type
char typeCheckPrint(Print print, char* fname, int lineNo, char* orig)
{
	return checkExpr(*(print.expression), fname, lineNo, orig, 1);
}

// Assign statements
char typeCheckAssign(Assign assign, char* fname, int lineNo, char* orig)
{
	// Left hand side must be a variable name OR a list index OR a field in object
	if (!(assign.lhs->type == VAR ||
		(assign.lhs->type == OP && assign.lhs->data.ope->type == IND) ||
		(assign.lhs->type == OP && assign.lhs->data.ope->type == MEMB)))
	{
		printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tCannot assign value to this expression.\n",
						fname, lineNo, orig);
		return 0;
	}

	// If it's a list index, check that we have a proper list and a proper index
	if (assign.lhs->type == OP && assign.lhs->data.ope->type == IND)
	{
		if (!checkExpr(*(assign.lhs), fname, lineNo, orig, 1)) return 0;
		return (
			verifyList(*(assign.lhs->data.ope->a), fname, lineNo, orig, 1) &&
			verifyInt(*(assign.lhs->data.ope->b), fname, lineNo, orig, 1)
			);
	}

	// If it's a member, check that we have a proper object
	if (assign.lhs->type == OP && assign.lhs->data.ope->type == MEMB)
	{
		if (!checkExpr(*(assign.lhs), fname, lineNo, orig, 1)) return 0;
		return verifyObj(*(assign.lhs->data.ope->a), fname, lineNo, orig, 1);
	}

	// Check the validity of the second expression
	if (!checkExpr(*(assign.rhs), fname, lineNo, orig, 1))
		return 0;

	return 1;
}

// For statements
char typeCheckFor(For fore, char* fname, int lineNo, char* orig)
{
	if (fore.iterator->type != VAR)
	{
		printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
			KRESET":\n\t" KCYN "%s" KRESET "\n\tIterator must be a variable.\n",
			fname, lineNo, orig);
	}

	if (!verifyList(*(fore.expression), fname, lineNo, orig, 1)) return 0;

	for (int i = 0; i  < fore.icount; i++)
		if (!typeCheckInstr(fore.instrs[i]))
			return 0;
	return 1;
}

// While statements
char typeCheckWhile(While whilee, char* fname, int lineNo, char* orig)
{
	if (!verifyBool(*(whilee.expression), fname, lineNo, orig, 1)) return 0;
	for (int i = 0; i  < whilee.icount; i++)
		if (!typeCheckInstr(whilee.instrs[i]))
			return 0;
	return 1;
}

// If statements
char typeCheckIf(If ifi, char* fname, int lineNo, char* orig)
{
	if (!verifyBool(*(ifi.expression), fname, lineNo, orig, 1)) return 0;
	for (int i = 0; i  < ifi.ificount; i++)
		if (!typeCheckInstr(ifi.ifinstrs[i]))
			return 0;
	for (int i = 0; i  < ifi.elicount; i++)
		if (!typeCheckInstr(ifi.elseinstrs[i]))
			return 0;
	return 1;
}

// Return statements
char typeCheckReturn(Return returni, char* fname, int lineNo, char* orig)
{
	return checkExpr(*(returni.expression), fname, lineNo, orig, 1);
}

// Check an expression is a boolean
char verifyBool(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == BOOL);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a boolean.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;
		
		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
			case NOT:
			case OR:
			case AND:
			case IS:
				return 1;
			case GT:
			case LT:
			case GTE:
			case LTE:
				return verifyNum(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyNum(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			case PLUS:
			case POW:
			case MULT:
			case MINUS:
			case DIV:
			case ONTO:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a boolean.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check an expression is a list
char verifyList(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == LIST || expr.data.conste->type == LISTCON);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a list.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;

		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
			case ONTO:
				return 1;
			case PLUS:
				return verifyList(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyList(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			case POW:
			case DIV:
			case MULT:
			case MINUS:
			case NOT:
			case GT:
			case LT:
			case GTE:
			case LTE:
			case IS:
			case OR:
			case AND:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a list.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check an expression is a string
char verifyStr(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == STR);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a string.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;
		
		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
				return 1;
			case PLUS:
			{
				char out = verifyStr(*(expr.data.ope->a), fname, lineNo, orig, 0) || verifyStr(*(expr.data.ope->b), fname, lineNo, orig, 0);
				if (!out && printIt)
				{
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a string.\n",
						fname, lineNo, orig);
				}
				return out;
			}
			case POW:
			case MULT:
			case MINUS:
			case DIV:
			case NOT:
			case ONTO:
			case GT:
			case LT:
			case GTE:
			case LTE:
			case IS:
			case OR:
			case AND:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a string.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check an expression is an object
char verifyObj(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == OBJ);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute an object.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;
		
		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
				return 1;
			case PLUS:
			case POW:
			case MULT:
			case MINUS:
			case DIV:
			case NOT:
			case ONTO:
			case GT:
			case LT:
			case GTE:
			case LTE:
			case IS:
			case OR:
			case AND:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute an object.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check an expression is a variable
char verifyVar(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == VAR)
		return 1;

	if (expr.type == OP)
	{
		if (checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt))
		{
			if (expr.data.ope->type==NOT || checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt))
			{
			
				switch (expr.data.ope->type)
				{
					case MEMB:
					case IND:
						return 1;
					case PLUS:
					case POW:
					case MULT:
					case MINUS:
					case DIV:
					case NOT:
					case ONTO:
					case GT:
					case LT:
					case GTE:
					case LTE:
					case IS:
					case OR:
					case AND:
					{
						break;
					}

				}
			}
		}
	}
	if (printIt)
		printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
					KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a name.\n",
					fname, lineNo, orig);
	return 0;
}

// Check an expression is an Integer
char verifyInt(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == INT);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute an integer.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;
		
		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
				return 1;
			case PLUS:
			case POW:
			case MULT:
			case MINUS:
			case DIV:
				return verifyInt(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyInt(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			case NOT:
			case ONTO:
			case GT:
			case LT:
			case GTE:
			case LTE:
			case IS:
			case OR:
			case AND:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute an integer.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check an expression is a Float
char verifyFloat(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		char out = (expr.data.conste->type == FLOAT);
		if (!out && printIt)
		{
			printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
				KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a float.\n",
				fname, lineNo, orig);
		}
		return out;
	}
	if (expr.type == OP)
	{
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;
		
		switch (expr.data.ope->type)
		{
			case MEMB:
			case IND:
				return 1;
			case PLUS:
			case POW:
			case MULT:
			case MINUS:
			case DIV:
			{
				if (!(verifyFloat(*(expr.data.ope->a), fname, lineNo, orig, 0) || verifyFloat(*(expr.data.ope->b), fname, lineNo, orig, 0)))
				{
					if (printIt)
						printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
							KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute an float.\n",
							fname, lineNo, orig);
					return 0;
				}
			}
			case NOT:
			case ONTO:
			case GT:
			case LT:
			case GTE:
			case LTE:
			case IS:
			case OR:
			case AND:
			{
				if (printIt)
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a float.\n",
						fname, lineNo, orig);
				return 0;
			}

		}
	}
	return 1;
}

// Check that an expression is numeric (ie Int or Float)
char verifyNum(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	char out =  (verifyFloat(expr, fname, lineNo, orig, 0) || 
			verifyInt(expr, fname, lineNo, orig, 0));
	if (!out && printIt)
	{
		printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
			KRESET":\n\t" KCYN "%s" KRESET "\n\tExpression does not constitute a numeric value.\n",
			fname, lineNo, orig);
	}
	return 1;
}

// Checking an Expression
char checkExpr(Expr expr, char* fname, int lineNo, char* orig, char printIt)
{
	if (expr.type == CONST)
	{
		// Constants are fine, unless they are lists,
		// in which case all sub items need to be checked
		if (expr.data.conste->type == LIST)
		{
			for (int i = 0; i < expr.data.conste->data.l.length; i++)
			{
				if (!checkExpr(expr.data.conste->data.l.values[i], fname, lineNo, orig, printIt))
					return 0;
			}
		}
		// For list constructors, all the parts need to be numeric
		if (expr.data.conste->type == LISTCON)
		{
			if (!verifyNum(*(expr.data.conste->data.lc.start), fname, lineNo, orig, printIt) ||
				!verifyNum(*(expr.data.conste->data.lc.end), fname, lineNo, orig, printIt) ||
				!verifyNum(*(expr.data.conste->data.lc.step), fname, lineNo, orig, printIt))
				return 0;
		}
		return 1;
	}
	if (expr.type == CALL)
	{
		// We need to check the arguments of call expressions
		for (int i = 0; i < expr.data.calle->argc; i++)
		{
			if (!checkExpr(expr.data.calle->args[i], fname, lineNo, orig, printIt))
				return 0;
		}
		return 1;
	}
	if (expr.type == OP)
	{
		// Check operation operands
		if (!checkExpr(*(expr.data.ope->a), fname, lineNo, orig, printIt)) return 0;
		if (expr.data.ope->type!=NOT)
			if (!checkExpr(*(expr.data.ope->b), fname, lineNo, orig, printIt)) return 0;

		// Specific operations require operands of specific types
		switch (expr.data.ope->type)
		{
			// If it's an IND, the first arg needs to be a list
			// And the second an integer
			case IND:
			{
				return verifyList(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyInt(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			}
			// Addition only works between numerics,
			// between lists, or between a string any anything else
			case PLUS:
			{
				char out =  (
							(verifyList(*(expr.data.ope->a), fname, lineNo, orig, 0) && verifyList(*(expr.data.ope->b), fname, lineNo, orig, 0)) ||
							(verifyNum(*(expr.data.ope->a), fname, lineNo, orig, 0) && verifyNum(*(expr.data.ope->b), fname, lineNo, orig, 0)) ||
							(verifyStr(*(expr.data.ope->a), fname, lineNo, orig, 0) || verifyStr(*(expr.data.ope->b), fname, lineNo, orig, 0))
						);
				if (!out && printIt)
				{
					printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
							KRESET":\n\t" KCYN "%s" KRESET "\n\tCannot add expressions of these types.\n",
							fname, lineNo, orig);
				}
				return out;
			}
			// Onto requires the second operand to be a list
			case ONTO:
			{
				return  verifyList(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			}
			// All of the following require two numeric operands
			case POW:
			case DIV:
			case MULT:
			case MINUS:
			case GT:
			case LT:
			case GTE:
			case LTE:
			{
				return verifyNum(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyNum(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			}
			case MEMB:
			{
				return verifyObj(*(expr.data.ope->a), fname, lineNo, orig, printIt) && verifyVar(*(expr.data.ope->b), fname, lineNo, orig, printIt);
			}
			// All of the others will automatically typecheck
			case NOT:
			case IS:
			case OR:
			case AND:
			{
				return 1;
			}

		}
	}
	return 1;
}