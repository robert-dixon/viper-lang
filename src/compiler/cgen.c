#include "includes/include.h"

// Function prototypes
void cGenFunction(BFunc fn, char** pos);
void cGenInstr(BInstr i, char** pos, char* buff);
char* escapeStr(char* str, char* buff);

int looplvl = 0;

// Turn the bytecode into C
char* cGen(BC* bc) {

	// Create the C code buffer
	char* out = malloc(MAX_C_PROG_LENGTH * sizeof(char));

	// Write the head of the file
	char* pos = out;
	strcpy(pos,
		"#include \"lib.h\"\n"
		"\n"
		"int vprtmpvals[8];\n"
		"\n"
		"int getArgC(int fid)\n"
		"{\n"
		"switch(fid)\n"
		"{\n"
	);
	pos += strlen(pos);

	char* str_buff = malloc(1024);
	for (int i = 0; i < bc->funcc; i++)
	{
		// Write out case statement
		sprintf(str_buff, "case %d:\n{\n", bc->functions[i].fid);
		strcpy(pos,str_buff);
		pos += strlen(pos);
		
		// Write out case statement
		sprintf(str_buff, "return %d;\n}\n", bc->functions[i].acount);
		strcpy(pos,str_buff);
		pos += strlen(pos);
	}

	free(str_buff);

	strcpy(pos,
		"default:\n"
		"{\n"
		"printf(KRED \"Error:\" KRESET \" Function does not exist!\\n\");\n"
		"return 0;\n"
		"}\n"
	);
	pos += strlen(pos);

	strcpy(pos,
		"}\n"
		"}\n"
		"int runFunc(int fid, int obj)\n"
		"{\n"
		"switch(fid)\n"
		"{\n"
	);
	pos += strlen(pos);

	// Write the C instructions
	for (int i = 0; i < bc->funcc; i++)
		cGenFunction(bc->functions[i], &pos);

	// Write the foot of the file
	strcpy(pos,
		"default:\n"
		"{\n"
		"printf(KRED \"Error:\" KRESET \" Function does not exist!\\n\");\n"
		"return 0;\n"
		"}\n"
		"}\n"
		"l_end();\n"
		"return 1;\n"
		"}"
	);
	pos += strlen(pos);

	// Write the buffer to a string of the correct length
	char* str = malloc((strlen(out) + 1) * sizeof(char));
	strcpy(str, out);
	free(out);
	return str;
}

// Functions are different cases (based on FID)
// In a switch statment
void cGenFunction(BFunc fn, char** pos)
{
	char* str_buff = malloc(8192);

	// Write out case statement
	sprintf(str_buff, "case %d:\n{\n", fn.fid);
	strcpy(*pos,str_buff);
	(*pos) += strlen(*pos);
	
	// Write instructions
	for (int i = 0; i < fn.instrc; i++)
		cGenInstr(fn.instructions[i], pos, str_buff);

	// End the case statement
	strcpy(*pos,"break;\n}\n");
	(*pos) += strlen(*pos);
	free(str_buff);
}

// Write the bytecode out as C
void cGenInstr(BInstr i, char** pos, char* buff)
{
	// Write instructions as C
	switch (i.type)
	{
		case BCONSTOBJ:
			{
				strcpy(*pos,"l_new();\n");
				break;
			}
		case BTHISOBJ:
			{
				strcpy(*pos,"l_push(obj);\n");
				break;
			}
		case BPRINT:
			{
				strcpy(*pos,"l_print(l_pop());\n");
				break;
			}
		case BCONSTFID:
			{
				sprintf(buff, "l_constfid(%s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BCONSTINT:
			{
				sprintf(buff, "l_constint(%s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BCONSTFLOAT:
			{
				sprintf(buff, "l_constfloat(%s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BCONSTBOOL:
			{
				sprintf(buff, "l_constbool(%s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BCONSTSTR:
			{
				sprintf(buff, "l_conststr(%s);\n", escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				break;
			}
		case BSTORE:
			{
				sprintf(buff, "l_store(l_pop(), %s);\n", escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				break;
			}
		case BLOAD:
			{
				sprintf(buff, "l_load(%s);\n", escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				break;
			}
		case BPLUS:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_plus(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BIND:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_ind(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BMEMB:
			{
				sprintf(buff, "l_memb(l_pop(), %s);\n", escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				break;
			}
		case BPOW:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_pow(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BMULT:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_mult(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BDIV:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_div(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BMINUS:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_minus(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BNOT:
			{
				strcpy(*pos,"l_not(l_pop());\n");
				break;
			}
		case BONTO:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_onto(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BGT:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_gt(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BLT:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_lt(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BGTE:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_gte(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BLTE:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_lte(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BIS:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_is(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BOR:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_or(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BAND:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_and(vprtmpvals[0], vprtmpvals[1]);\n");
				break;
			}
		case BRETURN:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_return(l_pop());\nreturn 1;\n");
				break;
			}
		case BCALL:
			{
				sprintf(buff, "l_call(obj, l_pop(), %s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BCALLM:
			{
				sprintf(buff, "vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_call(vprtmpvals[0], vprtmpvals[1], %s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BSQUASHLIST:
			{
				sprintf(buff, "l_squashlist(%s);\n", i.args[0]);
				strcpy(*pos,buff);
				break;
			}
		case BGENLIST:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nvprtmpvals[2] = l_pop();\nl_genlist(vprtmpvals[0], vprtmpvals[1], vprtmpvals[2]);\n");
				break;
			}
		case BIF:
			{
				strcpy(*pos,"if (l_if(l_pop())) {\n");
				(*pos) += strlen(*pos);

				for (int j = 0; j < i.ic; j++)
					cGenInstr(i.is[j], pos, buff);

				strcpy(*pos,"}\n");
				break;
			}
		case BWHILE:
			{
				strcpy(*pos,"while(1) {\n");
				(*pos) += strlen(*pos);

				for (int j = 0; j < i.ic; j++)
					cGenInstr(i.is[j], pos, buff);

				strcpy(*pos,"}\n");
				break;
			}
		case BFOREACH:
			{
				sprintf(buff, 
					"{\nint l%d = l_pop();\n"
				, looplvl);
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				sprintf(buff, 
					"if (getVal(l%d).type == LIST) {\n"
				, looplvl);
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				sprintf(buff, "for (int _i%d = 0; _i%d < getVal(l%d).data.l.length; _i%d++){\n",
					looplvl, looplvl, looplvl, looplvl);
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				sprintf(buff, "l_store(getVal(l%d).data.l.values[_i%d], %s);\n", looplvl, looplvl, escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				looplvl++;

				for (int j = 0; j < i.ic; j++)
					cGenInstr(i.is[j], pos, buff);

				looplvl--;

				strcpy(*pos,"}\n}\n}\n");
				break;
				break;
			}
		case BFOR:
			{
				sprintf(buff, 
					"{\nint c%d = l_pop();\n"
					"int b%d = l_pop();\n"
					"int a%d = l_pop();\n"
				, looplvl, looplvl, looplvl);
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				sprintf(buff, "for (int __%s = a%d; l_check_range(a%d,__%s,b%d); l_inc(__%s,c%d)) {\n",
					i.args[0], looplvl, looplvl, i.args[0], looplvl, i.args[0], looplvl);
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				sprintf(buff, "l_store(__%s, %s);\n", i.args[0], escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				(*pos) += strlen(*pos);

				looplvl++;

				for (int j = 0; j < i.ic; j++)
					cGenInstr(i.is[j], pos, buff);

				looplvl--;

				strcpy(*pos,"}\n}\n");
				break;
			}
		case BBREAK:
			{
				strcpy(*pos,"break;\n");
				break;
			}
		case BCONTINUE:
			{
				strcpy(*pos,"continue;\n");
				break;
			}
		case BYELSE:
			{
				strcpy(*pos,"else {\n");
				(*pos) += strlen(*pos);

				for (int j = 0; j < i.ic; j++)
					cGenInstr(i.is[j], pos, buff);

				strcpy(*pos,"}\n");
				break;
			}
		case BSETBASE:
			{
				strcpy(*pos,"l_setbase();\n");
				break;
			}
		case BUPDATELIST:
			{
				strcpy(*pos,"vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nvprtmpvals[2] = l_pop();\nl_updatelist(vprtmpvals[0], vprtmpvals[1], vprtmpvals[2]);\n");
				break;
			}
		case BUPDATEFIELD:
			{
				sprintf(buff, "vprtmpvals[0] = l_pop();\nvprtmpvals[1] = l_pop();\nl_updatefield(vprtmpvals[0], vprtmpvals[1], %s);\n",  escapeStr(i.args[0], buff));
				strcpy(*pos,buff);
				break;
			}
		case BPOP:
			{
				strcpy(*pos,"l_pop();\n");
				break;
			}
		default:
			{
				strcpy(*pos,"UNKNOWN\n");
				break;
			}
	}
	(*pos) += strlen(*pos);
}

// Escape string literals
char* escapeStr(char* str, char* buff)
{
	int spos = 0;
	int tpos = 0;
	buff[tpos++] = '"';
	while (str[spos] != 0)
	{
		if (str[spos] == '\n')
		{
			buff[tpos++] = '\\';
			buff[tpos++] = 'n';
		}
		else if (str[spos] == '\t')
		{
			buff[tpos++] = '\\';
			buff[tpos++] = 't';
		} else {
			if (str[spos] == '"') buff[tpos++] = '\\';
			if (str[spos] == '\\') buff[tpos++] = '\\';
			buff[tpos++] = str[spos];
		}
		spos++;
	}
	buff[tpos++] = '"';
	buff[tpos++] = 0;
	char* out = malloc(sizeof(char) * (tpos + 1));
	strcpy(out, buff);
	return out;
}