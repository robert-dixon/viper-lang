#include "includes/include.h"

// Function IDs need to be unique
int nextFid = 1;

// Parse an instruction
Instr* readInstr(char* line, char* takesBlock,
				char** error, char*** argList,
				int* aCount) {

	// Reset the takes block flag
	*takesBlock = 0;
	// This indicates whether the instruction
	// takes one or more blocks of further instructions
	// like an If statement, a For statement, or a function
	// declaration

	// Create the instruction
	Instr* out = malloc(sizeof(Instr));
	out->type = EMPTY; // Empty by default

	// Statements with Keywords//
	//=========================//
	// FOR statements
	if (strstr(line, "for ") == line) {
		out->type = FOR;
		line += 3; // skip past keyword

		// Check for "in" keyword
		char* list = strstr(line," in ");
		if (list == NULL)
		{
			*error = "Poorly constructed for statement.";
			return out;			
		}


		*list = 0; // end iterator string
		list += 4; // skip past 'in' keyword for list

		// Construct FOR instruction
		out->data.fori = malloc(sizeof(For));
		out->data.fori->icount = 0;
		out->data.fori->iterator = readExpr(line);
		out->data.fori->expression = readExpr(list);
		if (out->data.fori->iterator->type == NONE)
		{
			freeExpr(out->data.fori->iterator);
			*error = "Couldn't parse the iterator.";
			return out;	
		}
		if (out->data.fori->expression->type == NONE)
		{
			freeExpr(out->data.fori->expression);
			*error = "Couldn't parse the list expression.";
			return out;	
		}
		*takesBlock = 1; //FOR loops take block

	// IF statements
	} else if (strstr(line, "if ") == line) {
		// Construct IF instruction
		out->type = IF;
		*takesBlock = 1; //IF statements take blocks
		out->data.ifi = malloc(sizeof(If));
		out->data.ifi->ificount = 0;
		out->data.ifi->elicount = 0;
		//Skip past 'if' keyword
		out->data.ifi->expression = readExpr(line+3);
		if (out->data.ifi->expression->type == NONE)
		{
			freeExpr(out->data.ifi->expression);
			*error = "Couldn't parse condition.";
			return out;
		}
	// ELSE statements
	} else if (strstr(line, "else") == line) {
		*takesBlock = 2; // Secondary block for IF statement

		// Check that there are no words after the 'else'
		for (char* c = line+4; *c !=0; c++)
		{
			if (*c!=' ' && *c!='\t')
			{
				*error = "Invalid else statement.";
				break;
			}
		}
		return out;
	// CONTINUE statements
	} else if (strstr(line, "continue") == line) {
		// Check that there are no words after the 'continue'
		for (char* c = line+8; *c !=0; c++)
		{
			if (*c!=' ' && *c!='\t')
			{
				*error = "Invalid continue statement.";
				return out;
			}
		}

		// Construct the continue instruction
		out->type = CONTROL;
		out->data.controli = malloc(sizeof(Control));
		out->data.controli->type = CONTINUE;
		return out;
	// BREAK statements
	} else if (strstr(line, "break") == line) {
		// Check that there are no words after the 'break'
		for (char* c = line+5; *c !=0; c++)
		{
			if (*c!=' ' && *c!='\t')
			{
				*error = "Invalid break statement.";
				return out;
			}
		}

		// Construct the break instruction
		out->type = CONTROL;
		out->data.controli = malloc(sizeof(Control));
		out->data.controli->type = BREAK;
		return out;
	// WHILE statements
	} else if (strstr(line, "while ") == line) {
		*takesBlock = 1; // While statements take a block

		// Construct the While instruction
		out->type = WHILE;
		out->data.whilei = malloc(sizeof(While));
		out->data.whilei->icount = 0;
		// Skip past the keyword and parse the expression
		out->data.whilei->expression = readExpr(line+5);
		if (out->data.whilei->expression->type == NONE)
		{
			freeExpr(out->data.whilei->expression);
			*error = "Couldn't parse condition.";
		}
		return out;
	// RETURN statements
	}  else if (strstr(line, "return ") == line) {
		// Construct Return instruction
		out->type = RETURN;
		out->data.returni = malloc(sizeof(Return));
		// Skip past the keyword and parse the expression
		out->data.returni->expression = readExpr(line+6);
		if (out->data.returni->expression->type == NONE)
		{
			freeExpr(out->data.returni->expression);
			*error = "Couldn't parse return value.";
		}
		return out;

	// Statements without Keywords//
	//============================//
	} else {

		// Search for an '=' symbol
		char* eq = strchr(line, '=');
		char inString = 0;

		// Check that the '=' doesn't form part of a string
		// and doesn't for part of a '>='/'<=' operator
		// and is in fact assignment syntax
		if (eq!=NULL){
			char escape = 0;
			for (char* p = line; p < eq; p++)
			{
				if (*p == '"' && !escape) inString = !inString;
				if (*p == '\\' && !escape) {
					escape = 1;
				} else {
					escape = 0;
				}
			}
		}

		// If there is an assignment
		if (eq != NULL && *(eq-1) != '>' && *(eq-1) != '<' && !inString) {


			// Check if the line has an expression to the
			// left of the '=' symbol
			char has_lhs = (eq != line);

			// Check if there is an expression to the
			// right of the '=' symbol
			char has_rhs = 0;
			for (eq++; *eq != 0; eq++)
			{
				if (!(*eq == ' ' || *eq == '\t')) {
					has_rhs = 1;
					break;
				}
			}

			// LHS with no RHS
			// Function declaration
			if (!has_rhs && has_lhs){

				// Remove the '=' symbol
				eq = strchr(line, '=');
				*eq = 0;

				// Determine if line is preceeded by 'global'
				// keyword
				char g = 0;
				if (strstr(line, "global") == line)
				{
					g = 1;
					line += 7;

					// Skip past whitespace
					while (*line == ' ' || *line == '\t') line++;
				}

				char* name = line;

				// move forward to the first bit of whitespace 
				// after the function name
				while (*line != 0)
				{
					// characters
					while (*line != ' ' && *line != '\t' && *line != 0) line++;
					if (*line == 0) break;

					char* l = line;

					while ((*l == ' ' || *l != '\t') && *l != 0) l++;
					if (*l != 0 && (*l == ':' || *l == '.'))
					{
						line = l;
						continue;
					}

					*line = 0;
				}

				// Check the name contains only alphanumeric
				// characters, and that the first is alphabetical
				char okay = 1;
				char lhstype = 0;//0 = regular, 1 = member, 2 = index
				for (int j = 0; j < strlen(name); j++)
				{
					if (!(isalpha(name[j]) || (isdigit(name[j]) && j!=0) ||
						name[j] == ' ' || name[j] == '\t' || name[j] == ':' || name[j] == '.'))
					{
						okay = 0;
						break;
					}

					if (name[j] == '.' && lhstype != 2)
						lhstype = 1;
					if (name[j] == ':')
						lhstype = 2;
				}

				// Error if not
				if (!okay)
				{
					*error = "Invalid function name.";
					return out;
				}

				// Skip past any whitespace to the first argument
				line++;
				while (*line == ' ' || *line == '\t') line++;

				// Create a list of function arguments
				char** vars = malloc(sizeof(char*)*MAX_ARG_COUNT);
				int varc = 0; // Number of arguments

				// Start position of the current argument
				int oldStart = 0;

				// Create the list of arguments
				for (int i = 0; line[i] != 0; i++)
				{
					if (line[i] == ' ' || line[i] == '\t')
					{
						line[i] = 0;
						vars[varc++] = line + oldStart;

						oldStart = i + 1;
					}
				}

				if (strlen(line + oldStart) > 0)
				{
					vars[varc++] = line + oldStart;
				}

				// For each argument, check that it follows
				// proper naming rules as mentioned above
				for (int i = 0; i < varc; i++)
				{
					okay = 1;
					for (int j = 0; j < strlen(vars[i]); j++)
					{
						if (!(isalpha(vars[i][j]) || (isdigit(vars[i][j]) && j!=0)))
						{
							okay = 0;
							break;
						}
					}

					// Or error
					if (!okay)
					{
						*error = "Invalid argument.";
						return out;
					}
				}

				// Create the list of arguments, and copy them in
				*argList = malloc(sizeof(char*) * (varc + 1));

				if (lhstype == 0) {
					(*argList)[0] = malloc((strlen(name) + 1) * sizeof(char));
					strcpy((*argList)[0], name);
					*aCount = varc + 1;
				} else if (lhstype == 1) {
					(*argList)[0] = malloc((strlen("this") + 1) * sizeof(char));
					strcpy((*argList)[0], "this");
					*aCount = varc + 1;
				} else {
					*aCount = varc;
				}
				for (int i = 0; i < varc; i++)
				{
					(*argList)[i+1] = malloc((strlen(vars[i]) + 1) * sizeof(char));
					strcpy((*argList)[i+1], vars[i]);
				}

				// If it's global, add that back to the function name
				if (g)
				{
					char* newn = extraMemory((strlen(name) + 8) * sizeof(char));
					strcpy(newn, "global ");
					strcpy(newn + 7, name);
					name = newn;
				}
				
				// Create the FID to assign to the function variable
				Expr* exprl = readExpr(name);
				Expr* exprr = malloc(sizeof(Expr));
				exprr->type = CONST;
				exprr->data.conste = malloc(sizeof(Const));
				exprr->data.conste->type = FID;
				exprr->data.conste->data.fid = nextFid++;

				// Create Assignment instruction
				out->type = ASSIGN;
				out->data.assigni = malloc(sizeof(Assign));
				out->data.assigni->lhs = exprl;
				out->data.assigni->rhs = exprr;
				*takesBlock = (lhstype == 0 ? 3 : 4); // Special kind of block
				free(vars);
				return out;

			}
			// If there is a right-hand-side
			// Regular assignment
			else if (has_rhs)
			{	
				// Get the assignment symbol
				eq = strchr(line, '=');
				*eq = 0; //End the LHS
				eq++;

				// Get the two sides
				char* rhs = eq;
				char* lhs = line;

				// Create a LHS, if it's empty (assign to temporary variable)
				if (!has_lhs)
				{
					lhs = extraMemory(strlen("global __tmp") * sizeof(char));
					strcpy(lhs, "global __tmp");
				}

				// Create the Assignment instruction
				out->type = ASSIGN;
				out->data.assigni = malloc(sizeof(Assign));

				// Parse the ecpressions
				Expr* exprl = readExpr(lhs);
				Expr* exprr = readExpr(rhs);

				if (exprl->type == NONE)
				{
					freeExpr(exprl);
					*error = "Couldn't parse left hand side of assignment.";
					return out;
				}
				if (exprr->type == NONE)
				{
					freeExpr(exprr);
					*error = "Couldn't parse right hand side of assignment.";
					return out;
				}

				out->data.assigni->lhs = exprl;
				out->data.assigni->rhs = exprr;
				return out;	
			}
		// No keyword or assigment - must be an expression
		// An expression with no instruction is printed
		} else {
			// Create print statement
			out->type = PRINT;
			out->data.printi = malloc(sizeof(Print));
			// Parse expression
			out->data.printi->expression = readExpr(line);
			if (out->data.printi->expression->type == NONE)
			{
				freeExpr(out->data.printi->expression);
				*error = "Couldn't parse expression.";
			}
			return out;
		}
	}

	// We've no idea what's going on with this line
	if (out->type == EMPTY)
		*error = "Could not parse the line.";

	return out;
}

// Clear up memory
void freeInstr(Instr instr)
{
	if (instr.orig != NULL) free(instr.orig);
	switch (instr.type)
	{
		case PRINT:
		{
			freeExpr(instr.data.printi->expression);
			free(instr.data.printi);
			break;
		}
		case ASSIGN:
		{
			freeExpr(instr.data.assigni->lhs);
			freeExpr(instr.data.assigni->rhs);
			free(instr.data.assigni);
			break;
		}
		case WHILE:
		{
			freeExpr(instr.data.whilei->expression);
			for (int i = 0; i < instr.data.whilei->icount; i++)
				freeInstr(instr.data.whilei->instrs[i]);
			free(instr.data.whilei->instrs);
			free(instr.data.whilei);
			break;
		}
		case FOR:
		{
			freeExpr(instr.data.fori->iterator);
			freeExpr(instr.data.fori->expression);
			for (int i = 0; i < instr.data.fori->icount; i++)
				freeInstr(instr.data.fori->instrs[i]);
			free(instr.data.fori->instrs);
			free(instr.data.fori);
			break;
		}
		case IF:
		{
			freeExpr(instr.data.ifi->expression);
			for (int i = 0; i < instr.data.ifi->ificount; i++)
				freeInstr(instr.data.ifi->ifinstrs[i]);
			free(instr.data.ifi->ifinstrs);
			for (int i = 0; i < instr.data.ifi->elicount; i++)
				freeInstr(instr.data.ifi->elseinstrs[i]);
			if (instr.data.ifi->elicount != 0)
				free(instr.data.ifi->elseinstrs);
			free(instr.data.ifi);
			break;
		}
		case CONTROL:
		{
			free(instr.data.controli);
			break;
		}
		case RETURN:
		{
			freeExpr(instr.data.returni->expression);
			free(instr.data.returni);
			break;
		}
		default:
		{
			break;
		}
	}
}