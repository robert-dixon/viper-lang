#include "includes/include.h"

// Parse a string to strip out escape syntax
void parseStr(char* expr);
// Search for a particular operator expression
Expr* findOp(char* expr, char* searchFor, char unary, Operator opType, Expr* space);

// Parse an expression
Expr* readExpr(char* expr) {

	// Remove unhelpful whitespace
	expr = stripSpace(expr);

	// Create expression structure
	Expr* out = malloc(sizeof(Expr));
	out->type = NONE; //Default - doesn't work

	// Return empty expression if there's nothing left
	// after whitespace is gone
	if (strlen(expr) == 0) return out;

	// Constants //
	// --------- //

	// Empty Object
	if (strcmp(expr, "new") == 0)
	{
		// Create true boolean
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = OBJ;
		return out;
	}

	// Booleans
	if (strcmp(expr, "true") == 0)
	{
		// Create true boolean
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = BOOL;
		out->data.conste->data.b = 1;
		return out;
	}
	
	if (strcmp(expr, "false") == 0)
	{
		// Create false boolean
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = BOOL;
		out->data.conste->data.b = 0;
		return out;
	}

	// Numeric

	// Check if it is all digits with at most one point
	char allDigits = 1;
	char withPoint = 0;
	char start = (expr[0] == '-');
	for (int i = start; i < strlen(expr); i++) {
		if (!isdigit(expr[i]))
		{
			if (!withPoint && expr[i]=='.')
			{
				withPoint = 1;
			}
			else
			{
				allDigits = 0;
				break;
			}
		}
	}

	// No point
	if (allDigits && !withPoint)
	{
		// Create integer
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = INT;
		out->data.conste->data.v = atoi(expr);
		return out;
	}

	// With point
	if (allDigits && withPoint)
	{
		// Create float
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = FLOAT;
		out->data.conste->data.f = extraMemory((strlen(expr) + 1) * sizeof(char));
		strcpy(out->data.conste->data.f, expr);
		return out;
	}

	// String

	// Check the string starts and ends with "
	char allInStr = (expr[0] == '"' && expr[strlen(expr)-1] == '"');
	// Check the string isn't closed in the middle
	// eg, "a"+"b"
	if (allInStr)
	{
		char escape = 0;
		for (int i = 1; i < strlen(expr) - 1; i++) {
			if (expr[i] == '"' && !escape) {
				allInStr = 0;
				break;
			}
			if (expr[i] == '\\' && !escape) {
				escape = 1;
			} else {
				escape = 0;
			}
		}
	}

	// If it's all one legit string
	if (allInStr)
	{
		// Create the string
		parseStr(expr);
		out->type = CONST;
		out->data.conste = malloc(sizeof(Const));
		out->data.conste->type = STR;
		out->data.conste->data.str = extraMemory((strlen(expr)+1) * sizeof(char));
		strcpy(out->data.conste->data.str, expr);
		return out;
	}

	// Create a copy of the expression to manipulate
	char* cpy = malloc((strlen(expr) + 1) * sizeof(char));
	strcpy(cpy, expr);

	// Lists
	// If it starts with and ends with a '['
	if (expr[0] == '[' && expr[strlen(expr)-1] == ']')
	{
		// Check that it's all one list,
		// and not [1 2] + [3 4]
		int okay = 1;
		char llvl = 0;
		for (int i = 1; i < strlen(expr)-1; i++)
		{
			if (expr[i] == '[') llvl+=1;
			if (expr[i] == ']') llvl-=1;
			if (llvl < 0) {
				okay = 0;
				break;
			}
		}
		if (llvl != 0) okay = 0;

		// If it's all one list
		if (okay)
		{

			// Create the list
			out->type = CONST;
			out->data.conste = malloc(sizeof(Const));
			out->data.conste->type = LIST;
			out->data.conste->data.l.length = 0;

			// Get space separated list of values
			char* lc = cpy;
			lc[strlen(expr)-1] = 0;
			lc+=1;
			char* itmList = lc;
			int itmcount = 0;

			int next_pos = 0;
			int total_off = 0;

			int failed = 0;
			int hasItms = strlen(itmList) != 0;

			Expr** items = NULL;

			// If it's not an empty list
			if (hasItms)
			{
				items = malloc(sizeof(Expr*) * MAX_EXPLICIT_LIST_LEN);
				// Go through and parse all the list items
				while (1)
				{
					if (strlen(itmList) > 0)
					{
						// Try and get the largest expression possible
						Expr* checkExpr = readExpr(itmList);

						// If it failed to parse
						if (checkExpr->type == NONE)
						{
							free(checkExpr); // dispose of it

							// shorten the item to be attempted
							next_pos = 0;
							for (int i = strlen(itmList)-2; i >= -1; i--)
							{
								if (i == -1 || itmList[i] == ' ' || itmList[i] == '\t')
								{
									itmList[i] = 0;
									next_pos = i+1;
									break;
								}
							}
							if (next_pos == 0)
							{
								failed = 1;
								break;
							}

						// The item was legitimate
						} else {
							// Add it to the list of items
							items[itmcount++] = checkExpr;
							if (next_pos == 0)
							{
								break;
							}
							total_off += next_pos;
							next_pos = 0;
							strcpy(itmList, expr + 1 + total_off);
							itmList[strlen(itmList) - 1] = 0;
						}
					} else {
						break;
					}
				}
				// Add the parsed items to the list
				out->data.conste->data.l.length = itmcount;
				out->data.conste->data.l.values = malloc(itmcount * sizeof(Expr));
				for (int i = 0; i < itmcount; i++) out->data.conste->data.l.values[i] = *(items[i]);

			}
			return out;
		}
	}

	// List Constructors
	// Search for keyword
	char* sep = strstr(cpy, " to ");
	if (sep != NULL)
	{
		// Divide two halves of keyword
		*sep = 0;
		sep += 4;

		// Search for optional keyword in second half
		char* st = strstr(sep, " step ");
		if (st != NULL)
		{
			// Divide each side of keyword
			*st = 0;
			st += 6;			
		}

		// Make sure the left hand side all forms one
		// consistant expression
		int level = 0;
		char inString = 0;
		char escaped = 0;
		char okay = 1;
		for (int i = 0; cpy[i] != 0; i++)
		{
			if (cpy[i] == '(' && !inString) level += 1;
			if (cpy[i] == ')' && !inString) level -= 1;
			if (cpy[i] == '"' && !escaped) inString = !inString;
			if (cpy[i] == '\\' && !escaped){
				escaped = 1;
			} else {
				escaped = 0;
			}
			if (level < 0)
			{
				okay = 0;
				break;
			}
		}
		if (level != 0)
			okay = 0;

		// If it's consistant
		if (okay)
		{
			Expr* start = readExpr(cpy);
			Expr* end = readExpr(sep);

			// If there is no step value,
			// default it to 1
			Expr* step;
			if (st != NULL)
			{
				step = readExpr(st);
			}
			else
			{
				// Create '1' expression
				step = malloc(sizeof(Expr*));
				step->type = CONST;
				step->data.conste = malloc(sizeof(Const));
				step->data.conste->type = INT;
				step->data.conste->data.v = 1;
			}

			// If the start and end don't both parse, then this isn't
			// A valid list constructor
			if (start->type == NONE || end->type == NONE || step->type == NONE)
			{
				free(start);
				free(end);
				free(step);
				free(cpy);
				out->type = NONE;
				return out;
			}

			// Create the list constructor
			out->type = CONST;
			out->data.conste = malloc(sizeof(Const));
			out->data.conste->type = LISTCON;
			out->data.conste->data.lc.start = start;
			out->data.conste->data.lc.end = end;
			out->data.conste->data.lc.step = step;
			free (cpy);
			return out;
		}
	}

	// Reset copy of string, unmodified
	cpy = realloc(cpy, (strlen(expr) + 1) * sizeof(char));
	strcpy(cpy, expr);

	// Function calls
	char* c = cpy;
	// Check for 'global' keyword
	char* glob = strstr(c, "global ");
	char g = 0;
	if (glob != NULL)
	{
		g = 1;
		// Skip past keyword
		c += 7;
		// Skip past whitespace after keyword
		while (*c == ' ' || *c == '\t') c++;
	}

	// Check the name of the variable/function name
	// conforms to naming rules
	char* o = c;
	char valid = (isalpha(*c) || (*c == '_'));
	if (valid)
	{
		for (; *c!='!' && *c!=0; c++)
		{
			if (!(isalpha(*c) || (*c == '_') || (*c == ':') || (*c == '.') || isdigit(*c))) {
				valid = 0;
				break;
			}
		}
		if (*c == '!')
		{
			*c = 0;
			c++;
		} else {
			valid = 0;
		}
	}

	// Check all the arguments
	int argc = 0;
	char* argList = c;
	int coff = c - cpy;

	if (valid)
	{
		int argcount = 0;

		int next_pos = 0;
		int total_off = 0;

		int failed = 0;
		int hasArgs = strlen(argList) != 0;

		Expr** arguments = NULL;

		// If there are any to check
		if (hasArgs)
		{
			arguments = malloc(sizeof(Expr*) * MAX_ARG_COUNT);

			// Check through the potential arguments
			while (1)
			{
				if (strlen(argList) > 0)
				{
					// Check the largest possible argument
					Expr* checkExpr = readExpr(argList);

					//If it failed to parse
					if (checkExpr->type == NONE)
					{
						free(checkExpr);// Dispose of it
						// Shorten the argument to check
						next_pos = 0;
						for (int i = strlen(argList)-2; i >= -1; i--)
						{
							if (i == -1 || argList[i] == ' ' || argList[i] == '\t')
							{
								argList[i] = 0;
								next_pos = i+1;
								break;
							}
						}
						if (next_pos == 0)
						{
							failed = 1;
							break;
						}
					} else {
						// It parsed and was valid
						arguments[argcount++] = checkExpr;
						if (next_pos == 0)
							break;
						total_off += next_pos;
						next_pos = 0;
						// Add it to the list of arguments
						strcpy(argList, expr + coff + total_off);
					}
				} else {
					break;
				}
			}
		}

		// As long as we didn't totally fail to parse the
		// arguments
		if (!failed && (!hasArgs || argcount > 0))
		{
			// Construct the function call expression
			out->type = CALL;
			out->data.calle = malloc(sizeof(Call));

			char* lhsstr = malloc((strlen(o)+strlen(o)+(g?8:1))*sizeof(char));
			if (g)
				strcpy(lhsstr, "global.");
			strcpy(lhsstr+(g?7:0), o);

			out->data.calle->lhs = readExpr(lhsstr);
			free (lhsstr);

			out->data.calle->argc = argcount;
			out->data.calle->args = malloc(argcount * sizeof(Expr));
			for (int i = 0; i < argcount; i++)
				out->data.calle->args[i] = *(arguments[i]);
			free(cpy);
			return out;
		}
		if (arguments != NULL) free(arguments);
	}

	// Operations
	// Search for each expression in turn, and return it
	// If one is found
	Expr* op = findOp(expr, " and ", 0, AND, out);
	if (op != NULL) return op;
	op = findOp(expr, " or ", 0, OR, out);
	if (op != NULL) return op;
	op = findOp(expr, " is ", 0, IS, out);
	if (op != NULL) return op;
	op = findOp(expr, "<=", 0, LTE, out);
	if (op != NULL) return op;
	op = findOp(expr, ">=", 0, GTE, out);
	if (op != NULL) return op;
	op = findOp(expr, "<", 0, LT, out);
	if (op != NULL) return op;
	op = findOp(expr, ">", 0, GT, out);
	if (op != NULL) return op;
	op = findOp(expr, " onto ", 0, ONTO, out);
	if (op != NULL) return op;
	op = findOp(expr, "not ", 1, NOT, out);
	if (op != NULL) return op;
	op = findOp(expr, "+", 0, PLUS, out);
	if (op != NULL) return op;
	op = findOp(expr, "-", 0, MINUS, out);
	if (op != NULL) return op;
	op = findOp(expr, "/", 0, DIV, out);
	if (op != NULL) return op;
	op = findOp(expr, "*", 0, MULT, out);
	if (op != NULL) return op;
	op = findOp(expr, "^", 0, POW, out);
	if (op != NULL) return op;
	op = findOp(expr, ":", 0, IND, out);
	if (op != NULL) return op;
	op = findOp(expr, ".", 0, MEMB, out);
	if (op != NULL) return op;

	// Regular variables

	// Reset copy of string, unmodified
	cpy = realloc(cpy, (strlen(expr) + 1) * sizeof(char));
	strcpy(cpy, expr);
	c = cpy;

	// Check for global keyword
	glob = strstr(c, "global ");
	g = 0;
	if (glob != NULL)
	{
		g = 1;
		// Skip past keyword
		c += 7;
		// Skip past whitespace
		while (*c == ' ' || *c == '\t') c++;
	}

	o = c;

	// Check compliance with naming rules
	valid = (isalpha(*c) || (*c == '_'));
	if (valid)
	{
		for (; *c!=0; c++)
		{
			if (!(isalpha(*c) || (*c == '_') || isdigit(*c))) {
				valid = 0;
				break;
			}
		}
	}

	// If it complies
	if (valid)
	{
		// Construct the variable expression
		out->type = VAR;
		out->data.vare = malloc(sizeof(Var));
		out->data.vare->name = malloc((strlen(o)+(g?8:1))*sizeof(char));
		if (g)
			strcpy(out->data.vare->name, "global.");
		strcpy((out->data.vare->name)+(g?7:0), o);
		free(cpy);
		return out;	
	}

	free(cpy); // Dispose of the temporary copy of expression

	// Grouping
	if (expr[0] == '(' && expr[strlen(expr)-1] == ')')
	{
		// Grouping isn't an expression,
		// it's just to specify how they are nested
		// just search within the grouping
		expr[strlen(expr)-1] = 0;
		expr ++;
		return readExpr(expr);
	}

	// Error : Can't Parse
	return out;
}

// Strips the whitespace from the start and end of the expression
char* stripSpace(char* expr) {
	while (*expr == ' ' || *expr == '\t') expr++;
	int i;
	for (i = strlen(expr) - 1; i >= 0; i--)
		if (!(expr[i] == ' ' || expr[i] == '\t')) break;
	expr[i+1] = 0;
	return expr;
}

// Parses a string for its escape sequence
void parseStr(char* expr) {
	int rPos = 1;
	int wPos = 0;
	char escape = 0;
	while (expr[rPos] != 0)
	{
		char c = expr[rPos++];
		if (escape || (c != '"' && c!='\\'))
		{
			expr[wPos++] = c;
		}

		if (c == '"' && !escape) {
			expr[wPos++] = 0;
			return;
		}

		if (c == '\\' && !escape) {
			escape = 1;
		} else {
			escape = 0;
		}
	}
}

// Searches within an expression for a given operator and parses it
Expr* findOp(char* expr, char* searchFor, char unary, Operator opType, Expr* space)
{
	// Loop for operation keyword
	char* opPos = strstr(expr, searchFor);

	// if it's present
	if (opPos != NULL)
	{
		// Split on last one
		while (strstr(opPos + strlen(searchFor), searchFor) != NULL)
		{
			char* old = opPos;
			opPos = strstr(opPos + strlen(searchFor), searchFor);
		}
	}
	// if it's present
	if (opPos != NULL)
	{

		int level = 0;
		// binary operators
		if (!unary) {
			char inStr = 0;
			char escape = 0;
			// Check the operator is not part of a string
			// And check it's in the right grouping level
			// For this expression
			for (char* searchPos = expr; searchPos < opPos; searchPos++)
			{
				if (*searchPos == '\\' && !escape) {
					escape = 1;
				}
				else 
				{
					escape = 0;
				}

				if (*searchPos == '"' && !escape) inStr = !inStr;
				if (*searchPos == '(' && !inStr) level++;
				if (*searchPos == ')' && !inStr) level--;
			}
		}

		// If it's not in a string and it's the right
		// grouping level
		if (level == 0 && (!unary || opPos == expr))
		{
			// Create a temporary copy of the expression
			char* copy = malloc(strlen(expr)+1);
			strcpy(copy, expr);

			// Search for the operator
			opPos = strstr(copy, searchFor);

			// if it's present
			if (opPos != NULL)
			{
				// Split on last one
				while (strstr(opPos + strlen(searchFor), searchFor) != NULL)
				{
					char* old = opPos;
					opPos = strstr(opPos + strlen(searchFor), searchFor);
				}
			}

			// Split the two halves
			*opPos = 0;
			opPos += strlen(searchFor);

			// Parse each side
			Expr* vala = readExpr(unary ? opPos : copy);
			Expr* valb = (unary ? NULL : readExpr(opPos));

			free(copy); // Dispose of temporary copy

			// Make sure both sides parse
			if (vala->type == NONE || (!unary && valb->type == NONE))
				return NULL;

			// Create operator expression
			Expr* out = space;
			out->type = OP;
			out->data.ope = malloc(sizeof(Op));
			out->data.ope->type = opType;
			out->data.ope->a = vala;
			out->data.ope->b = valb;
			return out;
		}
	}
	// Fail to parse
	return NULL;
}

// Clear up memory
void freeExpr_(Expr expr);

void freeExpr(Expr* expr)
{
	if (expr != NULL)
	{
		freeExpr_(*expr);
		free(expr);
	}
}

void freeExpr_(Expr expr)
{
	switch (expr.type)
	{
		case CONST:
		{
			free(expr.data.conste);
			break;
		}
		case VAR:
		{
			free(expr.data.vare->name);
			free(expr.data.vare);
			break;
		}
		case OP:
		{
			if (expr.data.ope->a != NULL)
				freeExpr(expr.data.ope->a);
			if (expr.data.ope->a != NULL)
				freeExpr(expr.data.ope->b);
			free(expr.data.ope);
			break;
		}
		case CALL:
		{
			freeExpr(expr.data.calle->lhs);
			if (expr.data.calle->args != NULL)
			{
				for (int i = 0; i < expr.data.calle->argc; i++)
					freeExpr_(expr.data.calle->args[i]);
				free(expr.data.calle->args);
			}
			free(expr.data.calle);
			break;
		}
		default:
		{
			break;
		}
	}
}