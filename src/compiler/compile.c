#include "includes/include.h"

// Compiles the program from an AST
// To Bytecode

// Function Prototypes
BFunc compileFunction(Func fn);
void compileInstr(Instr instr, BInstr* ibfr, int* icount, char* strbfr);
void compileExpr(Expr expr, BInstr* ibfr, int* icount, char* strbfr);
void compileConst(Const conste, BInstr* ibfr, int* icount, char* strbfr);
void compileOp(Op op, BInstr* ibfr, int* icount, char* strbfr);

// Compile the whole program
BC* compileProgram(AST* prog) {
	// Create the bytecode structure
	BC* out = malloc(sizeof(BC));

	// Create a bytecode function for
	// each original function
	out->funcc = prog->funcc;
	out->functions = malloc(sizeof(BFunc) * out->funcc);
	for (int i = 0; i < out->funcc; i++)
		out->functions[i] = compileFunction(prog->functions[i]);

	return out;
}

// Create a bytecode function
BFunc compileFunction(Func fn) {

	// Create the structure
	BFunc out;

	out.acount = fn.acount;

	// Create a list of instructions
	BInstr* ibfr = malloc(sizeof(BInstr) * MAX_INSTRUCTION_LENGTH * 8);
	int icount = 0;

	// Create a temporary string buffer for manipulating
	char* strbfr = malloc(sizeof(char) * 1024);
	
	// If the function is normal
	if (fn.type == NORMAL)
	{
		out.acount -= 1;

		// Store the function ID as the first instruction
		ibfr[icount].type = BCONSTFID;
		ibfr[icount].argc = 1; 

		ibfr[icount].ic = 0;

		// Store the FID as a string
		sprintf(strbfr, "%d", fn.fid);
		ibfr[icount].args = malloc(sizeof(char*));

		ibfr[icount].args[0] = malloc((strlen(strbfr) + 1) * sizeof(char));
		strcpy(ibfr[icount].args[0], strbfr);

		icount++;

		// Add a store instruction as the second instruction
		// Attribute it to the name of this function so it can be
		// Accessed recursively
		ibfr[icount].type = BSTORE;
		ibfr[icount].argc = 1;
		ibfr[icount].ic = 0;

		ibfr[icount].args = malloc(sizeof(char*));

		ibfr[icount].args[0] = malloc((strlen(fn.args[0]) + 1) * sizeof(char));
		strcpy(ibfr[icount].args[0], fn.args[0]);

		icount++;

	}

	// If the function is a method
	if (fn.type == METHOD)
	{
		out.acount -= 1;

		// Store the object as the first argument
		ibfr[icount].type = BTHISOBJ;
		ibfr[icount].argc = 0; 
		ibfr[icount].ic = 0;

		icount++;

		// Add a store instruction as the second instruction
		ibfr[icount].type = BSTORE;
		ibfr[icount].argc = 1;
		ibfr[icount].ic = 0;

		ibfr[icount].args = malloc(sizeof(char*));

		ibfr[icount].args[0] = malloc((strlen(fn.args[0]) + 1) * sizeof(char));
		strcpy(ibfr[icount].args[0], fn.args[0]);

		icount++;

	}
	// For every argument, create a store instruction
	// To move it from the stack into variables
	for (int i = fn.acount; i > 1; i--)
	{
		ibfr[icount].type = BSTORE;
		ibfr[icount].argc = 1;
		ibfr[icount].ic = 0;

		ibfr[icount].args = malloc(sizeof(char*));

		ibfr[icount].args[0] = malloc((strlen(fn.args[i-1]) + 1) * sizeof(char));
		strcpy(ibfr[icount].args[0], fn.args[i-1]);

		icount++;
	}

	// Set the base of the function for the stack
	ibfr[icount].type = BSETBASE;
	ibfr[icount].argc = 0;
	ibfr[icount].ic = 0;

	icount++;

	for (int i = 0; i < fn.icount; i++)
		compileInstr(fn.instrs[i], ibfr, &icount, strbfr);

	out.fid = fn.fid;
	out.instrc = icount;
	out.instructions = malloc(icount * sizeof(BInstr));
	for (int i = 0; i < icount; i++)
	{
		out.instructions[i] = ibfr[i];
	}

	// Dispose of temporary structures
	free (ibfr);
	free (strbfr);
	return out;
}

// Compile an individual instruction
void compileInstr(Instr instr, BInstr* ibfr, int* icount, char* strbfr) {
	ibfr[*icount].ic = 0;
	switch (instr.type)
	{
		// Print instruction
		case PRINT:
			{
				// Compile the expression to print
				compileExpr(*(instr.data.printi->expression), ibfr, icount, strbfr);

				// Generate print instruction
				ibfr[*icount].type = BPRINT;
				ibfr[*icount].argc = 0;
				(*icount)++;

				return;
			}	
		// Assign instructions
		case ASSIGN:
			{
				// Compile the value
				compileExpr(*(instr.data.assigni->rhs), ibfr, icount, strbfr);
				ibfr[*icount].ic = 0;
				// Look at what is being assigned to
				switch (instr.data.assigni->lhs->type)
				{
					case VAR:
					{
						// If it's a variable, create store instruction for that variable
						char* n = instr.data.assigni->lhs->data.vare->name;
						ibfr[*icount].type = BSTORE;
						ibfr[*icount].argc = 1;

						ibfr[*icount].args = malloc(sizeof(char*));

						ibfr[*icount].args[0] = malloc((strlen(n) + 1) * sizeof(char));
						strcpy(ibfr[*icount].args[0], n);

						(*icount)++;
						break;
					}
					case OP:
					{
						// If it's an IND operation
						Op* ope = instr.data.assigni->lhs->data.ope;
						if (ope->type == IND)
						{
							// Compile the list and index values
							compileExpr(*(ope->a), ibfr, icount, strbfr);
							compileExpr(*(ope->b), ibfr, icount, strbfr);
							ibfr[*icount].type = BUPDATELIST;
							ibfr[*icount].argc = 0;
							ibfr[*icount].ic = 0;
							(*icount)++;

							break;
						}
						// If it's a MEMB
						if (ope->type == MEMB)
						{
							// Compile the object
							compileExpr(*(ope->a), ibfr, icount, strbfr);
							ibfr[*icount].type = BUPDATEFIELD;
							ibfr[*icount].argc = 1;
							ibfr[*icount].args = malloc(sizeof(char*));
							char* n = ope->b->data.vare->name;
							ibfr[*icount].args[0] = malloc((strlen(n) + 1) * sizeof(char));
							strcpy(ibfr[*icount].args[0], n);
							ibfr[*icount].ic = 0;
							(*icount)++;

							break;
						}
						break;
					}
					default:
						break;
				}

				return;
			}
		// Return instructions
		case RETURN:
			{
				// Just compile the expression to return
				compileExpr(*(instr.data.returni->expression), ibfr, icount, strbfr);

				// Then create return instruction
				ibfr[*icount].type = BRETURN;
				ibfr[*icount].argc = 0;
				ibfr[*icount].ic = 0;
				(*icount)++;

				return;
			}
		// If it's an If statement
		case IF:
			{
				// Compile the condition
				compileExpr(*(instr.data.ifi->expression), ibfr, icount, strbfr);
				ibfr[*icount].type = BIF; // Create IF instruction

				// Compile all the child instructions
				int ic = 0;
				BInstr* is = malloc(MAX_INSTRUCTION_LENGTH * 10 * sizeof(BInstr));
				for (int i = 0; i < instr.data.ifi->ificount; i++)
					compileInstr(instr.data.ifi->ifinstrs[i], is, &ic, strbfr);

				// Put those compiled instructions into the bytecode instruction
				ibfr[*icount].is = malloc (ic * sizeof(BInstr));
				for (int i = 0; i < ic; i++)
					ibfr[*icount].is[i] = is[i];
				
				ibfr[*icount].ic = ic;
				ibfr[*icount].argc = 0;

				// Create else instruction
				(*icount)++;
				ibfr[*icount].type = BYELSE;

				// Compile the else instruction
				ic = 0;
				for (int i = 0; i < instr.data.ifi->elicount; i++)
					compileInstr(instr.data.ifi->elseinstrs[i], is, &ic, strbfr);

				// Add the else children to the else instruction
				ibfr[*icount].is = malloc (ic * sizeof(BInstr));
				for (int i = 0; i < ic; i++)
					ibfr[*icount].is[i] = is[i];

				// Dispose of temporary list of instructions
				free(is);
				
				ibfr[*icount].ic = ic;
				ibfr[*icount].argc = 0;

				(*icount)++;
				return;
			}
		// While instruction
		case WHILE:
			{
				// While instruction
				ibfr[*icount].type = BWHILE;
				ibfr[*icount].argc = 0;

				int ic = 0;
				BInstr* is = malloc(MAX_INSTRUCTION_LENGTH * 10 * sizeof(BInstr));

				compileExpr(*(instr.data.whilei->expression), is, &ic, strbfr);

				// Instructions to exit while look when
				// condition no longer holds
				is[ic].type = BNOT;
				is[ic].argc = 0;
				is[ic].ic = 0;
				ic++;

				is[ic].type = BIF;
				is[ic].argc = 0;
				is[ic].ic = 1;
				is[ic].is = malloc(sizeof(BInstr));

				is[ic].is[0].type = BBREAK;
				is[ic].is[0].argc = 0;
				is[ic].is[0].ic = 0;

				ic++;

				// Compile child instructions
				for (int i = 0; i < instr.data.whilei->icount; i++)
					compileInstr(instr.data.whilei->instrs[i], is, &ic, strbfr);

				ibfr[*icount].is = malloc (ic * sizeof(BInstr));
				for (int i = 0; i < ic; i++)
					ibfr[*icount].is[i] = is[i];

				free(is);
				
				ibfr[*icount].ic = ic;

				(*icount)++;
				return;
			}

		// FOR statement
		case FOR:
			{
				if (instr.data.fori->iterator->type != VAR) return;

				if (instr.data.fori->expression->type == CONST &&
					instr.data.fori->expression->data.conste->type == LISTCON)
				{
					ListCon lc = instr.data.fori->expression->data.conste->data.lc;
					compileExpr(*(lc.start), ibfr, icount, strbfr);
					compileExpr(*(lc.end), ibfr, icount, strbfr);
					compileExpr(*(lc.step), ibfr, icount, strbfr);
					ibfr[*icount].ic = 0;

					ibfr[*icount].type = BFOR;

				} else {
					compileExpr(*(instr.data.fori->expression), ibfr, icount, strbfr);
					ibfr[*icount].type = BFOREACH;
				}

				ibfr[*icount].argc = 1;

				ibfr[*icount].args = malloc(sizeof(char*));
				char* iname = instr.data.fori->iterator->data.vare->name;
				ibfr[*icount].args[0] = malloc((strlen(iname) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], iname);

				int ic = 0;
				BInstr* is = malloc(MAX_INSTRUCTION_LENGTH * 10 * sizeof(BInstr));

				for (int i = 0; i < instr.data.fori->icount; i++)
					compileInstr(instr.data.fori->instrs[i], is, &ic, strbfr);

				ibfr[*icount].is = malloc (ic * sizeof(BInstr));
				for (int i = 0; i < ic; i++)
					ibfr[*icount].is[i] = is[i];

				free(is);
				
				ibfr[*icount].ic = ic;

				(*icount)++;
				return;
			}
		//CONTINUE, BREAK
		case CONTROL:
			{
				switch (instr.data.controli->type)
				{
					case BREAK:
					{
						ibfr[*icount].type = BBREAK;
						ibfr[*icount].argc = 0;
						ibfr[*icount].ic = 0;
						(*icount)++;
						break;
					}
					case CONTINUE:
					{
						ibfr[*icount].type = BCONTINUE;
						ibfr[*icount].argc = 0;
						ibfr[*icount].ic = 0;
						(*icount)++;
						break;
					}
				}
				return;
			}
		default:
			{
				return;
			}
	}
}

// Compile an individual expression and put evaluated value on stack
void compileExpr(Expr expr, BInstr* ibfr, int* icount, char* strbfr) {
	ibfr[*icount].ic = 0;
	switch (expr.type)
	{
		case CONST:
			{
				// Constants immediately resolve to constant values
				compileConst(*(expr.data.conste), ibfr, icount, strbfr);
				return;
			}
		case VAR:
			{
				// Load instruction, loads variable of that name
				ibfr[*icount].type = BLOAD;
				ibfr[*icount].argc = 1;

				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(expr.data.vare->name) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], expr.data.vare->name);
				(*icount)++;
				return;
			}
		case OP:
			{
				// Compile the operation
				compileOp(*(expr.data.ope), ibfr, icount, strbfr);
				return;
			}
		case CALL:
			{
				// Compile all of the arguments and leave them on the stack
				for (int i = 0; i < expr.data.calle->argc; i++)
				{
					compileExpr(expr.data.calle->args[i], ibfr, icount, strbfr);
				}

				// Load the FID associated with the function name
				compileExpr(*(expr.data.calle->lhs), ibfr, icount, strbfr);

				// Call that FID
				if (expr.data.calle->lhs->type == OP && expr.data.calle->lhs->data.ope->type == MEMB)
				{
					compileExpr(*(expr.data.calle->lhs->data.ope->a), ibfr, icount, strbfr);
					ibfr[*icount].type = BCALLM;
				}
				else
				{
					ibfr[*icount].type = BCALL;
				}
				ibfr[*icount].ic = 0;
				ibfr[*icount].argc = 1;

				sprintf(strbfr, "%d", expr.data.calle->argc);
				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(strbfr) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], strbfr);
				(*icount)++;

				return;
			}
		default:
			{
				return;
			}
	}
}

// Compiling specific operationg
void compileOp(Op op, BInstr* ibfr, int* icount, char* strbfr) {

	// Compile the operands
	compileExpr(*(op.a), ibfr, icount, strbfr);
	if (op.type != NOT && op.type != MEMB) compileExpr(*(op.b), ibfr, icount, strbfr);

	// Create the new instruction, one per operation
	ibfr[*icount].ic = 0;
	switch (op.type)
	{
		case IND:
		{
			ibfr[*icount].type = BIND;
			break;
		}
		case MEMB:
		{
			ibfr[*icount].type = BMEMB;
			ibfr[*icount].argc = 1;
			ibfr[*icount].args = malloc (sizeof(char*));
			ibfr[*icount].args[0] = malloc (sizeof(char) * (strlen(op.b->data.vare->name) + 1));
			strcpy(ibfr[*icount].args[0], op.b->data.vare->name);
			break;
		}
		case POW:
		{
			ibfr[*icount].type = BPOW;
			break;
		}
		case MULT:
		{
			ibfr[*icount].type = BMULT;
			break;
		}
		case DIV:
		{
			ibfr[*icount].type = BDIV;
			break;
		}
		case MINUS:
		{
			ibfr[*icount].type = BMINUS;
			break;
		}
		case PLUS:
		{
			ibfr[*icount].type = BPLUS;
			break;
		}
		case NOT:
		{
			ibfr[*icount].type = BNOT;
			break;
		}
		case ONTO:
		{
			ibfr[*icount].type = BONTO;
			break;
		}
		case GT:
		{
			ibfr[*icount].type = BGT;
			break;
		}
		case LT:
		{
			ibfr[*icount].type = BLT;
			break;
		}
		case GTE:
		{
			ibfr[*icount].type = BGTE;
			break;
		}
		case LTE:
		{
			ibfr[*icount].type = BLTE;
			break;
		}
		case IS:
		{
			ibfr[*icount].type = BIS;
			break;
		}
		case OR:
		{
			ibfr[*icount].type = BOR;
			break;
		}
		case AND:
		{
			ibfr[*icount].type = BAND;
			break;
		}
		default:
			break;
	}
	if (op.type != MEMB) ibfr[*icount].argc = 0;
	(*icount)++;
}

// Compiling the constants is a case of wrapping up the same values
// In the byte code structure.  No run-time evaluation is necessary
void compileConst(Const conste, BInstr* ibfr, int* icount, char* strbfr) {
	ibfr[*icount].ic = 0;
	switch (conste.type)
	{
		case OBJ:
			{
				ibfr[*icount].type = BCONSTOBJ;
				ibfr[*icount].argc = 0;
				(*icount)++;
				return;	
			}
		case BOOL:
			{
				ibfr[*icount].type = BCONSTBOOL;
				ibfr[*icount].argc = 1;

				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc(sizeof(char)*2);
				ibfr[*icount].args[0][0] = conste.data.b + '0';
				ibfr[*icount].args[0][1] = 0;
				(*icount)++;
				return;
			}
		case FLOAT:
			{
				ibfr[*icount].type = BCONSTFLOAT;
				ibfr[*icount].argc = 1;

				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(conste.data.f) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], conste.data.f);

				(*icount)++;
				return;
			}
		case INT:
			{
				ibfr[*icount].type = BCONSTINT;
				ibfr[*icount].argc = 1;

				sprintf(strbfr, "%d", conste.data.v);
				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(strbfr) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], strbfr);

				(*icount)++;
				return;
			}
		case STR:
			{
				ibfr[*icount].type = BCONSTSTR;
				ibfr[*icount].argc = 1;

				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(conste.data.str) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], conste.data.str);
				(*icount)++;
				return;
			}
		case FID:
			{
				ibfr[*icount].type = BCONSTFID;
				ibfr[*icount].argc = 1;

				sprintf(strbfr, "%d", conste.data.fid);
				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(strbfr) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], strbfr);

				(*icount)++;
				return;
			}
		case LIST:
			{
				List l = conste.data.l;

				for (int i = 0; i < l.length; i++)
				{
					compileExpr(l.values[i], ibfr, icount, strbfr);
				}

				ibfr[*icount].ic = 0;
				ibfr[*icount].type = BSQUASHLIST;
				ibfr[*icount].argc = 1;

				sprintf(strbfr, "%d", l.length);
				ibfr[*icount].args = malloc(sizeof(char*));

				ibfr[*icount].args[0] = malloc((strlen(strbfr) + 1) * sizeof(char));
				strcpy(ibfr[*icount].args[0], strbfr);

				(*icount)++;

				return;
			}
		case LISTCON:
			{

				ListCon lc = conste.data.lc;

				compileExpr(*(lc.start), ibfr, icount, strbfr);
				compileExpr(*(lc.end), ibfr, icount, strbfr);
				compileExpr(*(lc.step), ibfr, icount, strbfr);
				ibfr[*icount].ic = 0;

				ibfr[*icount].type = BGENLIST;
				ibfr[*icount].argc = 0;

				(*icount)++;

				return;
			}
		default:
			{
				return;
			}
	}
}


// Clear up memory
void freeBInstr(BInstr instr)
{
	for (int i = 0; i < instr.ic; i++)
		freeBInstr (instr.is[i]);
	if (instr.ic != 0) free (instr.is);
	for (int i = 0; i < instr.argc; i++)
		free (instr.args[i]);
	if (instr.argc != 0) free (instr.args);
}

void freeBC(BC* bc)
{
	for (int i = 0; i < bc->funcc; i++)
	{
		for (int j = 0; j < bc->functions[i].instrc; j++)
		{
			freeBInstr(bc->functions[i].instructions[j]);
		}
		free(bc->functions[i].instructions);
	}
	free(bc->functions);
	free(bc);
}