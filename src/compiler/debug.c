#include "includes/include.h"
#define INDENT {for (int idnt = 0; idnt < indent; idnt++) printf("   ");}

// Dumps program structures out to the console

// Function Prototypes
void dumpFunci(Func function, int indent);
void dumpInstri(Instr instruction, int indent);
void dumpPrinti(Print printinstr, int indent);
void dumpAssigni(Assign assigninstr, int indent);
void dumpIfi(If ifinstr, int indent);
void dumpReturni(Return retinstr, int indent);
void dumpControli (Control ctrlinstr, int indent);
void dumpFori(For forinstr, int indent);
void dumpExpri(Expr expression, int indent);
void dumpConsti(Const constant, int indent);
void dumpOpi(Op operation, int indent);
void dumpVari(Var variable, int indent);
void dumpCalli(Call funccall, int indent);
void dumpWhilei(While whileinstr, int indent);


char* getBInstr(BInstrType t);
void dumpBInstr(BInstr instr, int indent);

void dumpAST(AST* ast)
{
	printf(KRED "AST" KRESET " (len=%d)\n", ast->funcc);
	for (int i = 0; i < ast->funcc; i++)
	{
		dumpFunci(ast->functions[i], 1);
	}
}

void dumpFunci(Func function, int indent)
{
	INDENT;
	printf(KYEL "FUNCTION" KRESET " (fid=%d, len=%d, args=[" KCYN, function.fid, function.icount);
	for (int i = 0; i < function.acount-1; i++)
		printf("%s, ", function.args[i]);
	if (function.acount > 0) printf("%s", function.args[function.acount-1]);
	printf(KRESET "])\n");
	for (int i = 0; i < function.icount; i++)
	{
		dumpInstri(function.instrs[i], indent+1);
	}
}

void dumpInstri(Instr instruction, int indent)
{
	INDENT;
	printf(KRED "INSTRUCTION" KRESET " (type=%s)\n", strIType(instruction.type));
	switch(instruction.type)
	{
		case PRINT:
		{
			dumpPrinti(*(instruction.data.printi), indent+1);
			break;
		}
		case ASSIGN:
		{
			dumpAssigni(*(instruction.data.assigni), indent+1);
			break;
		}
		case RETURN:
		{
			dumpReturni(*(instruction.data.returni), indent+1);
			break;
		}
		case CONTROL:
		{
			dumpControli(*(instruction.data.controli), indent+1);
			break;
		}
		case FOR:
		{
			dumpFori(*(instruction.data.fori), indent+1);
			break;
		}
		case IF:
		{
			dumpIfi(*(instruction.data.ifi), indent+1);
			break;
		}
		case WHILE:
		{
			dumpWhilei(*(instruction.data.whilei), indent+1);
			break;
		}
		default:
			break;
	}
}

void dumpPrinti(Print printinstr, int indent)
{
	INDENT;
	printf(KRED "PRINT" KRESET "\n");
	dumpExpri(*(printinstr.expression), indent+1);
}

void dumpAssigni(Assign assigninstr, int indent)
{
	INDENT;
	printf(KRED "ASSIGN" KRESET "\n");
	indent++;
	INDENT;
	printf(KCYN "left-hand-side:" KRESET "\n");
	dumpExpri(*(assigninstr.lhs), indent+1);
	INDENT;
	printf(KCYN "right-hand-side:" KRESET "\n");
	dumpExpri(*(assigninstr.rhs), indent+1);
}

void dumpFori(For forinstr, int indent)
{
	INDENT;
	printf(KRED "FOR" KRESET "\n");
	indent++;
	INDENT;
	printf(KCYN "iterator:" KRESET "\n");
	dumpExpri(*(forinstr.iterator), indent+1);
	INDENT;
	printf(KCYN "condition:" KRESET "\n");
	dumpExpri(*(forinstr.expression), indent+1);
	INDENT;
	printf(KCYN "loop:" KRESET "(len=%d)\n", forinstr.icount);
	for (int i = 0; i < forinstr.icount; i++)
	{
		dumpInstri(forinstr.instrs[i], indent+1);
	}
}

void dumpControli(Control ctrlinstr, int indent)
{
	INDENT;
	printf(KRED "CONTROL" KRESET " : ");
	switch (ctrlinstr.type)
	{
		case BREAK:
			printf("BREAK\n");
			break;
		case CONTINUE:
			printf("CONTINUE\n");
			break;
		default:
			printf("OTHER\n");
			break;
	}
	printf("\n");
}

void dumpReturni(Return retinstr, int indent)
{
	INDENT;
	printf(KRED "RETURN" KRESET "\n");
	dumpExpri(*retinstr.expression, indent+1);
}

void dumpIfi(If ifinstr, int indent)
{
	INDENT;
	printf(KRED "IF" KRESET "\n");
	indent++;
	INDENT;
	printf(KCYN "condition:" KRESET "\n");
	dumpExpri(*(ifinstr.expression), indent+1);
	INDENT;
	printf(KCYN "if true:" KRESET "(len=%d)\n", ifinstr.ificount);
	for (int i = 0; i < ifinstr.ificount; i++)
	{
		dumpInstri(ifinstr.ifinstrs[i], indent+1);
	}
	INDENT;
	printf(KCYN "else:" KRESET "(len=%d)\n", ifinstr.elicount);
	for (int i = 0; i < ifinstr.elicount; i++)
	{
		dumpInstri(ifinstr.elseinstrs[i], indent+1);
	}
}

void dumpWhilei(While whileinstr, int indent)
{
	INDENT;
	printf(KRED "WHILE" KRESET "\n");
	indent++;
	INDENT;
	printf(KCYN "condition:" KRESET "\n");
	dumpExpri(*(whileinstr.expression), indent+1);
	INDENT;
	printf(KCYN "loop:" KRESET "(len=%d)\n", whileinstr.icount);
	for (int i = 0; i < whileinstr.icount; i++)
	{
		dumpInstri(whileinstr.instrs[i], indent+1);
	}
}

void dumpExpri(Expr expression, int indent)
{
	INDENT;
	printf(KRED "EXPR" KRESET " (type=%s)\n", strEType(expression.type));
	switch(expression.type)
	{
		case CONST:
		{
			dumpConsti(*(expression.data.conste), indent+1);
			break;
		}
		case OP:
		{
			dumpOpi(*(expression.data.ope), indent+1);
			break;
		}
		case VAR:
		{
			dumpVari(*(expression.data.vare), indent+1);
			break;
		}
		case CALL:
		{
			dumpCalli(*(expression.data.calle), indent+1);
			break;
		}
		default:
			break;
	}
}

void dumpConsti(Const constant, int indent)
{
	INDENT;
	printf(KRED "CONSTANT" KRESET "(type=%s)\n", strCType(constant.type));
	indent += 1;
	INDENT;
	switch(constant.type)
	{
		case OBJ:
		{
			printf(KRED "NEW OBJECT" KRESET);
			break;
		}
		case STR:
		{
			printf(KRED "STR" KRESET ": \"%s\"", constant.data.str);
			break;
		}
		case BOOL:
		{
			printf(KRED "BOOL" KRESET ": %s", (constant.data.b ? "true" : "false"));
			break;
		}
		case INT:
		{
			printf(KRED "INT" KRESET ": %d", constant.data.v);
			break;
		}
		case FID:
		{
			printf(KRED "FID" KRESET ": %d", constant.data.fid);
			break;
		}
		case FLOAT:
		{
			printf(KRED "FLOAT" KRESET ": %s", constant.data.f);
			break;
		}
		case LIST:
		{
			printf(KRED "LIST" KRESET);
			for (int i = 0; i < constant.data.l.length; i++)
			{
				printf("\n");
				dumpExpri(constant.data.l.values[i], indent+1);
			}
			break;
		}
		case LISTCON:
		{
			printf(KRED "LIST" KRESET "\n");
			indent += 1;
			INDENT;
			printf(KCYN "start:\n" KRESET);
			dumpExpri(*constant.data.lc.start, indent+1);
			INDENT;
			printf(KCYN "end:\n" KRESET);
			dumpExpri(*constant.data.lc.end, indent+1);
			INDENT;
			printf(KCYN "step:\n" KRESET);
			dumpExpri(*constant.data.lc.step, indent+1);
			break;
		}
		default:
			break;
	}
	printf("\n");
}

void dumpOpi(Op operation, int indent)
{
	INDENT;
	printf(KRED "OPERATION" KRESET " (type=%s)\n", strOType(operation.type));
	dumpExpri(*(operation.a), indent+1);
	if (operation.type != NOT)
		dumpExpri(*(operation.b), indent+1);
}

void dumpVari(Var variable, int indent)
{
	INDENT;
	printf(KRED "VARIABLE" KRESET " (name=%s)\n", variable.name);
}

void dumpCalli(Call funccall, int indent)
{
	INDENT;
	printf(KRED "FUNCTION CALL" KRESET " (args=%d)\n", funccall.argc);
	indent += 1;
	INDENT;
	printf(KCYN "function:\n" KRESET);
	dumpExpri(*(funccall.lhs), indent+1);
	INDENT;
	printf(KCYN "arguments:\n" KRESET);
	for (int i = 0; i < funccall.argc; i++)
		dumpExpri(funccall.args[i], indent + 1);
}

// Wrappers for non-indented forms
void dumpFunc(Func function)
{
	dumpFunci(function, 0);
}
void dumpInstr(Instr instruction)
{
	dumpInstri(instruction, 0);
}
void dumpPrint(Print printinstr)
{
	dumpPrinti(printinstr, 0);
}
void dumpAssign(Assign assigninstr)
{
	dumpAssigni(assigninstr, 0);
}
void dumpIf(If ifinstr)
{
	dumpIfi(ifinstr, 0);
}
void dumpFor(For forinstr)
{
	dumpFori(forinstr, 0);
}
void dumpControl(Control ctrlinstr)
{
	dumpControli(ctrlinstr, 0);
}
void dumpReturn(Return retinstr)
{
	dumpReturni(retinstr, 0);
}
void dumpWhile(While whileinstr)
{
	dumpWhilei(whileinstr, 0);
}
void dumpExpr(Expr expression)
{
	dumpExpri(expression, 0);
}
void dumpConst(Const constant)
{
	dumpConsti(constant, 0);
}
void dumpOp(Op operation)
{
	dumpOpi(operation, 0);
}
void dumpVar(Var variable)
{
	dumpVari(variable, 0);
}
void dumpCall(Call funccall)
{
	dumpCalli(funccall, 0);
}

// Convert ENUMs to strings
char* strIType(IType t)
{
	switch (t)
	{
		case EMPTY:
			return "EMPTY";
		case PRINT:
			return "PRINT";
		case ASSIGN:
			return "ASSIGN";
		case WHILE:
			return "WHILE";
		case FOR:
			return "FOR";
		case IF:
			return "IF";
		case CONTROL:
			return "CONTROL";
		case RETURN:
			return "RETURN";
		default:
			return "OTHER";
	}
}

char* strEType(EType t)
{
	switch (t)
	{
		case NONE:
			return "NONE";
		case CONST:
			return "CONST";
		case VAR:
			return "VAR";
		case OP:
			return "OP";
		case CALL:
			return "CALL";
		default:
			return "OTHER";
	}
}

char* strCType(CType t)
{
	switch (t)
	{
		case UNDEF:
			return "UNDEF";
		case INT:
			return "INT";
		case STR:
			return "STR";
		case FLOAT:
			return "FLOAT";
		case BOOL:
			return "BOOL";
		case FID:
			return "FID";
		case LIST:
			return "LIST";
		case LISTCON:
			return "LISTCON";
		case OBJ:
			return "OBJ";
		default:
			return "OTHER";
	}
}

char* strOType(Operator t)
{
	switch (t)
	{
		case IND:
			return "IND";
		case POW:
			return "POW";
		case MULT:
			return "MULT";
		case DIV:
			return "DIV";
		case MINUS:
			return "MINUS";
		case PLUS:
			return "PLUS";
		case NOT:
			return "NOT";
		case ONTO:
			return "ONTO";
		case GT:
			return "GT";
		case LT:
			return "LT";
		case GTE:
			return "GTE";
		case LTE:
			return "LTE";
		case IS:
			return "IS";
		case OR:
			return "OR";
		case AND:
			return "AND";
		case MEMB:
			return "MEMB";
		default:
			return "OTHER";
	}
}

//BC
void dumpBC(BC* bc) {
	printf(KRED "BYTE CODE\n" KRESET);
	for (int i = 0; i < bc->funcc; i++)
	{
		BFunc fn = bc->functions[i];
		int indent = 1;
		INDENT;
		printf(KYEL "FUNCTION" KRESET ": %d (len=%d)\n", fn.fid, fn.instrc);
		for (int j = 0; j < fn.instrc; j++)
		{
			dumpBInstr(fn.instructions[j], 2);
		}
	}
}

void dumpBInstr(BInstr instr, int indent)
{
	INDENT;
	printf(KRED "%s" KRESET, getBInstr(instr.type));
	for (int k = 0; k < instr.argc; k++)
	{
		printf("\t%s", instr.args[k]);
	}
	printf("\n");
	for (int i = 0; i < instr.ic; i++)
	{
		dumpBInstr(instr.is[i], indent+1);
	}
}

char* getBInstr(BInstrType t)
{
	switch (t)
	{
		case BPRINT:
			return "PRINT      ";
		case BCONSTFID:
			return "CONSTFID   ";
		case BCONSTINT:
			return "CONSTINT   ";
		case BCONSTFLOAT:
			return "CONSTFLOAT ";
		case BCONSTBOOL:
			return "CONSTBOOL  ";
		case BCONSTSTR:
			return "CONSTSTR   ";
		case BCONSTOBJ:
			return "CONSTOBJ   ";
		case BSTORE:
			return "STORE      ";
		case BLOAD:
			return "LOAD       ";
		case BPLUS:
			return "PLUS       ";
		case BIND:
			return "IND        ";
		case BPOW:
			return "POW        ";
		case BMULT:
			return "MULT       ";
		case BDIV:
			return "DIV        ";
		case BMINUS:
			return "MINUS      ";
		case BNOT:
			return "NOT        ";
		case BONTO:
			return "ONTO       ";
		case BGT:
			return "GT         ";
		case BLT:
			return "LT         ";
		case BGTE:
			return "GTE        ";
		case BLTE:
			return "LTE        ";
		case BIS:
			return "IS         ";
		case BOR:
			return "OR         ";
		case BAND:
			return "AND        ";
		case BMEMB:
			return "MEMB       ";
		case BRETURN:
			return "RETURN     ";
		case BCALL:
			return "CALL       ";
		case BCALLM:
			return "CALLM      ";
		case BSQUASHLIST:
			return "SQUASHLIST ";
		case BGENLIST:
			return "GENLIST    ";
		case BIF:
			return "IF         ";
		case BWHILE:
			return "WHILE      ";
		case BFOREACH:
			return "FOREACH    ";
		case BFOR:
			return "FOR        ";
		case BBREAK:
			return "BREAK      ";
		case BCONTINUE:
			return "CONTINUE   ";
		case BYELSE:
			return "ELSE       ";
		case BTHISOBJ:
			return "THISOBJ    ";
		case BSETBASE:
			return "SETBASE    ";
		case BUPDATELIST:
			return "BUPDATELIST";
		case BUPDATEFIELD:
			return "BUPDATEFIELD";
		case BPOP:
			return "BPOP       ";
		default:
			return "UNKNOWN    ";
	}
}