#include <limits.h>
#include "lib.h"
#include <math.h>
#include <ctype.h>

// PRIMITIVE FUNCTIONS
typedef enum {
	P_START,
	P_DEF,
	P_READ,
	P_TOINT,
	P_TOFLOAT,
	P_TOSTR,
	P_LOAD,
	P_EXIT,
	P_SAVE,
	P_LENGTH,
	P_TOLOWER,
	P_TOUPPER,
	P_STRPOS,
	P_SUBSTR,
	P_SPLIT,
	P_ERROR,
	P_SANDWICH,
	P_END
} PrimFn;


// Prototypes
void sprintVal(char* target, VAL v);
void cleanUp();
int saveToSpace(VAL o);
int updateInSpace(int ptr, VAL o);
char compVals(VAL v2, VAL v1);
char compareLists(VAL l1, VAL l2);
char compareObjs(VAL o1, VAL o2);
VAL emptyObj();
float diff();
void clear_f_vars();
void runPrim(PrimFn fn);
char l_check_range_v(VAL l, VAL a, VAL u);
void l_inc_v(VAL* it, VAL inc);
void setUpPrims();
void setUpPrim();

VAL* user_space = NULL;
int user_space_end = 0;

int nullptr_p = 0;
int nullobj_p = 0;

int fdepth = 0;

int* stackbase = NULL;
int stack_len = 0;
int* stack = NULL;

char* str_buff;

char** varlist;
int* vallist;
int varC = 0;

// Difference between two numeric values
float diff(VAL end, VAL start)
{
	if (start.type == INT)
	{
		if (end.type == INT)
		{
			return (float)(end.data.v - start.data.v);
		}
		return (float)(end.data.f - start.data.v);
	}
	if (end.type == INT)
	{
		return (float)(end.data.v - start.data.f);
	}
	return (float)(end.data.f - start.data.f);
}

// Create function names for all the primatives
void setUpPrims()
{
	setUpPrim(INT_MAX-P_DEF, "def");
	setUpPrim(INT_MAX-P_READ, "read");
	setUpPrim(INT_MAX-P_TOINT, "toInt");
	setUpPrim(INT_MAX-P_TOFLOAT, "toFloat");
	setUpPrim(INT_MAX-P_TOSTR, "toStr");
	setUpPrim(INT_MAX-P_LOAD, "load");
	setUpPrim(INT_MAX-P_SAVE, "save");
	setUpPrim(INT_MAX-P_TOLOWER, "toLower");
	setUpPrim(INT_MAX-P_TOUPPER, "toUpper");
	setUpPrim(INT_MAX-P_STRPOS, "strPos");
	setUpPrim(INT_MAX-P_SUBSTR, "subStr");
	setUpPrim(INT_MAX-P_SPLIT, "split");
	setUpPrim(INT_MAX-P_SANDWICH, "sandwich");
	setUpPrim(INT_MAX-P_LENGTH, "length");
	setUpPrim(INT_MAX-P_ERROR, "error");
	setUpPrim(INT_MAX-P_EXIT, "exit");
}

// Store that primative in the variable table
void setUpPrim(int fid, char* name)
{
	VAL v;
	v.type = FID;
	v.data.fid = fid;
	vallist[varC] = saveToSpace(v);
	varlist[varC] = malloc((strlen(name) + 1) * sizeof(char));
	strcpy(varlist[varC], name);
	varC++;
}

// Start the program
int main(int argc, char **argv) {
	// Set up structures
	str_buff = malloc(sizeof(char) * MAX_STRING_LENGTH);
	varlist = malloc(sizeof(char*) * MAX_VAR_COUNT);
	vallist = malloc(sizeof(VAL) * MAX_VAR_COUNT);

	// create null pointer
	VAL nullptr;
	nullptr.type = UDEF;
	nullptr_p = saveToSpace(nullptr);

	// create null object
	nullobj_p = saveToSpace(emptyObj());

	// Setup up primitives
	setUpPrims();

	// Add the commandline argument list

	VAL args;
	args.type = LIST;
	args.data.l.length = argc;
	args.data.l.values = malloc (argc * sizeof(int));
	for (int i = 0; i < argc; i++)
	{
		VAL sobj;
		sobj.type = STR;
		sobj.data.str = argv[i];
		args.data.l.values[i] = saveToSpace(sobj);
	}
	
	l_store(saveToSpace(args), "global.args");
	
	fdepth++;
	runFunc(0, nullobj_p);
	cleanUp();
}

// Clear the function-local variables when they're out of scope
void clear_f_vars()
{
	sprintf(str_buff,"f%d-", fdepth);
	for (int i = 0; i < varC; i++)
	{
		if (varlist[i] == NULL) continue;
		if (strstr(varlist[i], str_buff) == varlist[i]) {
			free(varlist[i]);
			varlist[i] = NULL;
		}
	}
}

// Run a primative function
void runPrim(PrimFn fn)
{
	switch(fn)
	{
		// Is a variable defined
		case P_DEF:
		{
			// Load the variable and see if it's "undefined"
			VAL v = getVal(l_pop());
			VAL bobj;
			bobj.type = BOOL;
			bobj.data.b = v.type != UDEF;
			int ptr = saveToSpace(bobj);
			l_push(ptr);	
			break;
		}
		// Read from console input
		case P_READ:
		{
			char *readline = malloc(1024*8);
			fgets(readline, 1024*8, stdin);

			VAL sobj;		
			sobj.data.str = malloc(sizeof(char) * (strlen(readline) + 1));
			strcpy(sobj.data.str, readline);
			sobj.data.str[strlen(readline) - 1] = 0;
			sobj.type = STR;
			free(readline);

			int ptr = saveToSpace(sobj);
			l_push(ptr);
			break;
		}
		// Convert a value to string
		case P_TOSTR:
		{
			VAL v = getVal(l_pop());
			sprintVal(str_buff, v);
			VAL o;
			o.type = STR;
			o.data.str = malloc(sizeof(char) * (strlen(str_buff) + 1));
			strcpy(o.data.str, str_buff);
			l_push(saveToSpace(o));
			break;
		}
		// Convert a value to an integer
		case P_TOINT:
		{
			VAL v = getVal(l_pop());

			// Depends what kind of value it is
			switch(v.type)
			{
				// Read strings
				case STR:
				{
					l_constint(atoi(v.data.str));
					break;
				}
				// Leave ints
				case INT:
				{
					l_push(saveToSpace(v));
					break;
				}
				// Floor floats
				case FLOAT:
				{
					l_constint(floor(v.data.f));
					break;
				}
				default:
				{
					// Error if it can't be converted
					l_constbool(0);
					printf(KRED "Error:" KRESET " Cannot convert value to integer.\n");
					exit(EXIT_FAILURE);
				}
			}
			break;
		}
		// Save thing for floats
		case P_TOFLOAT:
		{
			VAL v = getVal(l_pop());
			switch(v.type)
			{
				// Read Strings
				case STR:
				{
					l_constfloat(atof(v.data.str));
					break;
				}
				// Convert Ints
				case INT:
				{
					l_constfloat(v.data.v * 1.0);
					break;
				}
				// Loave Floats
				case FLOAT:
				{
					l_push(saveToSpace(v));
					break;
				}
				default:
				{
					// Error if it's a different type
					l_constbool(0);
					printf(KRED "Error:" KRESET " Cannot convert value to float.\n");
					exit(EXIT_FAILURE);
				}
			}
			break;
		}
		// End the program
		case P_EXIT:
		{
			exit(EXIT_SUCCESS);
		}
		// Load a file from a file path
		case P_LOAD:
		{
			VAL v = getVal(l_pop());
			if (v.type != STR)
			{
				printf(KRED "Error:" KRESET " File path is not a string.\n");
				exit(EXIT_FAILURE);
			}

			FILE *file;
			file = fopen(v.data.str, "r");

			// Return 'false' to stack if it failed
			if (!file)
			{
				l_constbool(0);
				break;
			}

			// Otherwise, return the file contents as a string
			int pos = 0;
			while (1) {
				char chr = fgetc(file);
				if (chr == EOF) break;
				str_buff[pos++] = chr;
			}
			fclose(file);
			str_buff[pos] = 0;

			char* str = malloc(sizeof(char) * (strlen(str_buff) + 1));
			strcpy(str, str_buff);
			l_conststr(str);
			break;
		}

		// Save a file to a file name
		case P_SAVE:
		{
			VAL s = getVal(l_pop());
			VAL v = getVal(l_pop());

			// Error if the path is not a string
			if (v.type != STR)
			{
				printf(KRED "Error:" KRESET " File path is not a string.\n");
				exit(EXIT_FAILURE);
			}

			FILE *file;
			file = fopen(v.data.str, "wb");

			// Return 'false' to stack on failure
			if (!file)
			{
				l_constbool(0);
				break;
			}

			// Write the value (after converting it to a string)
			switch(s.type)
			{
				case STR:
				{
					fputs(s.data.str, file);
					l_constbool(1);
					break;
				}
				case BOOL:
				{
					fputs(s.data.b? "true" : "false", file);
					l_constbool(1);
					break;
				}
				case INT:
				{
					sprintf(str_buff, "%d", s.data.v);
					fputs(str_buff, file);
					l_constbool(1);
					break;
				}
				case FLOAT:
				{
					sprintf(str_buff, "%.3f", s.data.f);
					fputs(str_buff, file);
					l_constbool(1);
					break;
				}
				default:
				{
					printf(KRED "Error:" KRESET " Cannot save a value of this type to file.\n");
					l_constbool(0);
					exit(EXIT_FAILURE);
				}
			}
			fclose(file);
			break;
		}
		// Convert String to upper case
		case P_TOUPPER:
		{
			VAL v = getVal(l_pop());

			// Must be string
			if (v.type !=STR)
			{
				printf(KRED "Error:" KRESET " Cannot convert a non-string value to upper case.\n");
				exit(EXIT_FAILURE);
			}


			VAL n;
			n.type = STR;
			n.data.str = malloc ((strlen(v.data.str) + 1) * sizeof(char));
			strcpy(n.data.str, v.data.str);
			char* c = n.data.str;

			while (*c != 0)
			{
				*c = toupper(*c);
				c++;
			}

			l_push(saveToSpace(n));

			break;
		}

		// Find the position of a substring in a string
		case P_STRPOS:
		{
			VAL sub = getVal(l_pop());
			VAL search = getVal(l_pop());

			// Both arguments must be strings
			if (search.type !=STR || sub.type != STR)
			{
				printf(KRED "Error:" KRESET " Can only use the 'strPos' function on string values.\n");
				exit(EXIT_FAILURE);
			}

			// Return false it it's not found
			if (strstr(search.data.str, sub.data.str) == NULL)
			{
				l_constbool(0);
			}
			// Otherwise return the index
			else
			{
				int ind = strstr(search.data.str, sub.data.str) - search.data.str;
				l_constint(ind);
			}
			break;
		}
		// Find a substring inside a string
		case P_SUBSTR:
		{
			VAL len = getVal(l_pop());
			VAL start = getVal(l_pop());
			VAL str = getVal(l_pop());

			// Check argument types
			if (len.type != INT || start.type != INT || str.type != STR)
			{
				printf(KRED "Error:" KRESET " Can only use the 'subStr' function with integer index and length on a string value.\n");
				exit(EXIT_FAILURE);
			}

			if (len.data.v < 0)
			{
				printf(KRED "Error:" KRESET " Length cannot be negative in 'subStr'.\n");
				exit(EXIT_FAILURE);
			}

			if (start.data.v + len.data.v > strlen(str.data.str))
			{
				printf(KRED "Error:" KRESET " Index for 'subStr' is out of range.\n");
				exit(EXIT_FAILURE);
			}

			// Return the substring
			VAL s;
			s.type = STR;
			s.data.str = malloc(sizeof(char) * (len.data.v  + 1));
			for (int i = 0; i < len.data.v; i++)
			{
				s.data.str[i] = str.data.str[start.data.v + i];
			}
			s.data.str[len.data.v] = 0;
			l_push(saveToSpace(s));
			break;
		}
		// Convert a string to lower case
		case P_TOLOWER:
		{
			VAL v = getVal(l_pop());

			// Must be string value
			if (v.type !=STR)
			{
				printf(KRED "Error:" KRESET " Cannot convert a non-string value to lower case.\n");
				exit(EXIT_FAILURE);
			}

			VAL n;
			n.type = STR;
			n.data.str = malloc ((strlen(v.data.str) + 1) * sizeof(char));
			strcpy(n.data.str, v.data.str);
			char* c = n.data.str;

			while (*c != 0)
			{
				*c = tolower(*c);
				c++;
			}

			l_push(saveToSpace(n));
			break;
		}
		// Split a string to a list on a certain delimiter
		case P_SPLIT:
		{
			VAL delim = getVal(l_pop());
			VAL str = getVal(l_pop());

			// Check types are string
			if (delim.type !=STR || str.type != STR)
			{
				printf(KRED "Error:" KRESET " Split function can only be applied to strings.\n");
				exit(EXIT_FAILURE);
			}

			// Split the string up
			char* st = malloc ((strlen(str.data.str) + 1) * sizeof(char));
			strcpy(st, str.data.str);

			int count = 1;

			char** subs = malloc(sizeof(char*) * strlen(st));
			subs[0] = st;

			while (strstr(st, delim.data.str) != NULL)
			{
				st = strstr(st, delim.data.str);
				*st = 0;
				st += strlen(delim.data.str);
				subs[count++] = st;
			}

			// Return list
			VAL out;
			out.type = LIST;
			out.data.l.length = count;
			out.data.l.values = malloc(sizeof(INT) * count);
			for (int i = 0; i < count; i++)
			{
				VAL strobj;
				strobj.type = STR;
				strobj.data.str = subs[i];
				int ptr = saveToSpace(strobj);
				out.data.l.values[i]= ptr;
			}
			free (subs);
			l_push(saveToSpace(out));
			break;
		}
		// Turn a list of strings into a string
		case P_SANDWICH:
		{
			VAL delim = getVal(l_pop());
			VAL list = getVal(l_pop());

			// Check we have a list and a string type
			if (delim.type !=STR || list.type != LIST)
			{
				printf(KRED "Error:" KRESET " Sandwich function can only be applied on a string and a list.\n");
				exit(EXIT_FAILURE);
			}

			char* str = malloc(MAX_STRING_LENGTH * sizeof(char));

			char* p = str;

			// Add the word to the end of the string
			for (int i = 0; i < list.data.l.length; i++)
			{
				sprintVal(str_buff, getVal(list.data.l.values[i]));
				strcpy(p, str_buff);
				p+= strlen(str_buff);
				if (i != list.data.l.length - 1) {
					strcpy(p, delim.data.str);
					p+=strlen(delim.data.str);
				}
			}

			// Return the string
			char* ostr = malloc((strlen(str) + 1)* sizeof(char));
			strcpy(ostr, str);
			l_conststr(ostr);
			break;
		}

		// Get length of list or string
		case P_LENGTH:
		{
			VAL v = getVal(l_pop());
			
			switch(v.type)
			{
				case STR:
				{
					l_constint(strlen(v.data.str));
					break;
				}
				case LIST:
				{
					l_constint(v.data.l.length);
					break;
				}
				default:
				{
					printf(KRED "Error:" KRESET " Value type not associated with a length.\n");
					l_constbool(0);
					exit(EXIT_FAILURE);
				}
			}
			break;
		}

		case P_ERROR:
		{
			VAL v = getVal(l_pop());
			l_constbool(0);
			if (v.type == STR)
			{
				char* str = v.data.str;
				printf(KRED "Error:" KRESET " %s\n", str);

			} else {
				printf(KRED "Error" KRESET "\n");
			}
			exit(EXIT_FAILURE);
		}

		// Unimplemented primative
		default:
		{
			printf(KRED "Error:" KRESET " Unrecognised primitive function.\n");
			exit(EXIT_FAILURE);
		}
	}
}

void cleanUp()
{
	if (stackbase != NULL) free(stackbase);
	if (stack != NULL) free(stack);
	if (user_space != NULL) free(user_space);
	if (str_buff != NULL) free(str_buff);
	for (int i = 0; i < varC; i++) free(varlist[varC]);
	if (varlist != NULL) free(varlist);
	if (vallist != NULL) free(vallist);
}

// Store a value in memory
int saveToSpace(VAL o)
{
	if (user_space == NULL)
		user_space = malloc(sizeof(VAL) * MAX_SPACE);
	user_space[user_space_end++] = o;
	return user_space_end-1;
}

int updateInSpace(int ptr, VAL o)
{
	if (user_space == NULL)
		user_space = malloc(sizeof(VAL) * MAX_SPACE);
	user_space[ptr] = o;
	return ptr;
}

// Store a value in memory
VAL getVal(int ptr)
{
	if (user_space == NULL)
		user_space = malloc(sizeof(VAL) * MAX_SPACE);

	return user_space[ptr];
}

// Operations
int l_pop()
{
	if (stack == NULL)
		stack = malloc(sizeof(int)*MAX_STACK_SIZE);

	if (stack_len > 0)
	{
		stack_len--;
		return stack[stack_len];
	} else {
		return nullptr_p;
	}
}
void l_push(int ptr)
{
	if (stack == NULL)
		stack = malloc(sizeof(int)*MAX_STACK_SIZE);

	stack[stack_len] = ptr;
	stack_len++;
}

void l_print(int v)
{
	sprintVal(str_buff, getVal(v));
	printf("%s\n", str_buff);
}
void l_conststr(char* str)
{
	VAL strobj;
	strobj.type = STR;
	strobj.data.str = str;
	int ptr = saveToSpace(strobj);
	l_push(ptr);
}
void l_constfid(int fid)
{
	VAL fobj;
	fobj.type = FID;
	fobj.data.fid = fid;
	int ptr = saveToSpace(fobj);
	l_push(ptr);
}
void l_constint(int v)
{
	VAL iobj;
	iobj.type = INT;
	iobj.data.v = v;
	int ptr = saveToSpace(iobj);
	l_push(ptr);
}
void l_constfloat(float f)
{
	VAL fobj;
	fobj.type = FLOAT;
	fobj.data.f = f;
	int ptr = saveToSpace(fobj);
	l_push(ptr);	
}
void l_constbool(char b)
{
	VAL bobj;
	bobj.type = BOOL;
	bobj.data.b = b;
	int ptr = saveToSpace(bobj);
	l_push(ptr);		
}
void l_store(int p, char* name)
{
	char global = 0;
	if (strstr(name, "global.") == name)
	{
		global = 1;
		name += 7;
	}
	*str_buff = 0;
	if (global) {
		sprintf(str_buff,"%s", name);
	} else {
		sprintf(str_buff,"f%d-%s", fdepth, name);
	}

	for (int i = 0; i < varC; i++)
	{
		if (varlist[i] == NULL) continue;
		if (strcmp(varlist[i], str_buff) == 0)
		{
			vallist[i] = p;
			return;
		}
	}
	for (int i = 0; i < varC; i++)
	{
		if (varlist[i] == NULL) {
			vallist[i] = p;
			varlist[i] = malloc((strlen(str_buff) + 1) * sizeof(char));
			strcpy(varlist[i], str_buff);
			return;
		}
	}
	vallist[varC] = p;
	varlist[varC] = malloc((strlen(str_buff) + 1) * sizeof(char));
	strcpy(varlist[varC], str_buff);
	varC++;
}

void l_load(char* name)
{
	char global = 0;
	if (strstr(name, "global.") == name)
	{
		global = 1;
		name += 7;
	}

	*str_buff = 0;
	if (global) {
		sprintf(str_buff,"%s", name);
	} else {
		sprintf(str_buff,"f%d-%s", fdepth, name);
	}	

	for (int i = 0; i < varC; i++)
	{
		if (varlist[i] == NULL) continue;
		if (strcmp(varlist[i], str_buff) == 0)
		{
			l_push(vallist[i]);
			return;
		}
	}
	for (int i = 0; i < varC; i++)
	{
		if (varlist[i] == NULL) continue;
		if (strcmp(varlist[i], name) == 0)
		{
			l_push(vallist[i]);
			return;
		}
	}
	VAL out;
	out.type = UDEF;
	l_push(saveToSpace(out));
}

void l_call(int o, int ptr, int argc)
{
	VAL v = getVal(ptr);
	if (v.type != FID)
	{
		printf(KRED "Error:" KRESET " Attempting to call non-function variable.\n");
		exit(EXIT_FAILURE);
	}
	int chk = INT_MAX - v.data.fid;
	if (chk > P_START && chk < P_END)
	{
		runPrim(chk);
	}
	else if (argc != getArgC(v.data.fid))
	{
		printf(KRED "Error:" KRESET " Function called with incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	} else {
		fdepth++;
		runFunc(v.data.fid, o);
	}
}
void l_not(int p)
{
	VAL v = getVal(p);

	VAL obj;
	obj.data.b = (v.type == BOOL && v.data.b == 0);
	obj.type = BOOL;
	l_push(saveToSpace(obj));
}
char l_if(int p)
{
	VAL v = getVal(p);
	return !(v.type == BOOL && v.data.b == 0);
}
void l_plus(int v2p, int v1p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL nval;
	nval.type = UDEF;

	// Loads of different operations depending on
	// types of operands
	switch (v1.type)
	{
		case UDEF:
		{
			nval = v2;
			break;
		}
		case LIST:
		{
			switch (v2.type)
			{
				case UDEF:
				{
					nval = v1;
					break;
				}
				case LIST:
				{
					nval.type = LIST;
					nval.data.l.length = v1.data.l.length + v2.data.l.length;
					nval.data.l.values = malloc(sizeof(int) * nval.data.l.length);
					for (int i = 0 ; i < v1.data.l.length; i++)
					{
						nval.data.l.values[i] = v1.data.l.values[i];
					}
					for (int i = 0 ; i < v2.data.l.length; i++)
					{
						nval.data.l.values[i + v1.data.l.length] = v2.data.l.values[i];
					}
					break;
				}
				case STR:
				{
					sprintVal(str_buff, v1);
					
					nval.type = STR;
					nval.data.str = malloc(sizeof(char) * (strlen(str_buff) + strlen(v2.data.str) + 1));
					sprintf(nval.data.str, "%s%s", str_buff, v2.data.str);
					break;
				}
				default:
					break;
			}
			break;
		}
		case INT:
		{
			switch (v2.type)
			{
				case UDEF:
				{
					nval = v1;
					break;
				}
				case INT:
				{
					nval.type = INT;
					nval.data.v = v1.data.v + v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.v + v2.data.f;
					break;
				}
				case STR:
				{
					nval.type = STR;
					sprintf(str_buff, "%d%s", v1.data.v, v2.data.str);
					nval.data.str = malloc(sizeof(char) * (strlen(str_buff) + 1));
					strcpy(nval.data.str, str_buff);
					break;
				}
				default:
					break;
			}
			break;
		}
		case BOOL:
		{
			switch (v2.type)
			{
				case UDEF:
				{
					nval = v1;
					break;
				}
				case STR:
				{
					nval.type = STR;
					sprintf(str_buff, "%s%s", v1.data.b ? KGRN "true" KRESET : KRED "false" KRESET, v2.data.str);
					nval.data.str = malloc(sizeof(char) * (strlen(str_buff) + 1));
					strcpy(nval.data.str, str_buff);
					break;
				}
				default:
					break;
			}
			break;
		}
		case FLOAT:
		{
			switch (v2.type)
			{
				case UDEF:
				{
					nval = v1;
					break;
				}
				case INT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f + v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f + v2.data.f;
					break;
				}
				case STR:
				{
					nval.type = STR;
					sprintf(str_buff, "%.3f%s", v1.data.f, v2.data.str);
					nval.data.str = malloc(sizeof(char) * (strlen(str_buff) + 1));
					strcpy(nval.data.str, str_buff);
					break;
				}
				default:
					break;
			}
			break;
		}
		case STR:
		{
			sprintVal(str_buff, v2);
			char* v2s = malloc(sizeof(char) * (strlen(str_buff) + 1));
			strcpy(v2s, str_buff);
			nval.type = STR;
			sprintf(str_buff, "%s%s", v1.data.str, v2s);
			nval.data.str = malloc(sizeof(char) * (strlen(str_buff) + 1));
			strcpy(nval.data.str, str_buff);
			break;
		}
		default:
		{
			if (v2.type == UDEF) nval = v1;
			break;
		}
	}

	l_push(saveToSpace(nval));

	if (nval.type == UDEF)
	{
		printf(KRED "Error:" KRESET " Adding expressions of incompatible types.\n");
		exit(EXIT_FAILURE);
	}
	
}
void l_minus(int v2p, int v1p)
{

	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL nval;
	nval.type = UDEF;

	switch (v1.type)
	{
		case INT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = INT;
					nval.data.v = v1.data.v - v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.v - v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		case FLOAT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f - v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f - v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	l_push(saveToSpace(nval));

	if (nval.type == UDEF)
	{
		printf(KRED "Error:" KRESET " '-' operation not valid on non-numeric operands.\n\n");
		exit(EXIT_FAILURE);
	}
}

void l_is(int v1p, int v2p)
{

	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL obj;
	obj.data.b = compVals(v1, v2);
	obj.type = BOOL;
	l_push(saveToSpace(obj));
	
}

// Compare values for equality
char compVals(VAL v2, VAL v1)
{
	char eq = (v1.type != FLOAT);
	eq = eq && (v1.type == v2.type);
	eq = eq && ((v1.type != INT) || (v1.data.v == v2.data.v));
	eq = eq && ((v1.type != BOOL) || (v1.data.b == v2.data.b));
	eq = eq && ((v1.type != FID) || (v1.data.fid == v2.data.fid));
	eq = eq && ((v1.type != STR) || (strcmp(v1.data.str, v2.data.str) == 0));
	eq = eq && ((v1.type != LIST) || compareLists(v1,v2));
	eq = eq && ((v1.type != OBJ) || compareObjs(v1,v2));

	return eq;
}

// Compare lists for equality
char compareLists(VAL l1, VAL l2)
{
	if (l1.type != LIST || l2.type != LIST) return 0;

	if (l1.data.l.length != l2.data.l.length) return 0;

	for (int i = 0; i < l1.data.l.length; i++)
		if (!compVals(getVal(l1.data.l.values[i]), getVal(l2.data.l.values[i]))) return 0;

	return 1;
}

// Compare objects for equality
char compareObjs(VAL o1, VAL o2)
{
	if (o1.type != LIST || o2.type != OBJ) return 0;

	if (o1.data.o.length != o2.data.o.length) return 0;

	for (int i = 0; i < o1.data.o.length; i++)
	{
		if (!compVals(getVal(o1.data.o.values[i]), getVal(o2.data.o.values[i]))) return 0;
		if (strcmp(o1.data.o.fields[i], o2.data.o.fields[i])) return 0;
	}

	return 1;
}


void l_or(int v1p, int v2p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL obj;
	obj.data.b = (v1.type == BOOL && v1.data.b) || (v2.type == BOOL && v2.data.b);
	obj.type = BOOL;
	l_push(saveToSpace(obj));

}
char l_check_range(int lp, int ap, int up)
{
	VAL a = getVal(ap);
	VAL l = getVal(lp);
	VAL u = getVal(up);

	return l_check_range_v(l, a, u);
}
void l_inc(int itp, int incp)
{
	VAL inc = getVal(incp);
	VAL* it = user_space + itp;
	
	return l_inc_v(it, inc);
}
// Check that a value is within a given range of values
char l_check_range_v(VAL l, VAL a, VAL u)
{
	if (!((a.type == INT || a.type == FLOAT) && (l.type == INT || l.type == FLOAT) && (u.type == INT || u.type == FLOAT)))
	{
		printf(KRED "Error:" KRESET " Non-numeric values used in list constructor.\n");
		exit(EXIT_FAILURE);
	}
	if (l.type == INT)
	{
		if (a.type == INT)
		{
			if (u.type == INT)
				return a.data.v <= u.data.v && a.data.v >= l.data.v;
			
			return a.data.v <= u.data.f && a.data.v >= l.data.v;
		}
		if (u.type == INT)
			return a.data.f <= u.data.v && a.data.f >= l.data.v;
			
		return a.data.f <= u.data.f && a.data.f >= l.data.v;
	}
	if (a.type == INT)
	{
		if (u.type == INT)
			return a.data.v <= u.data.v && a.data.v >= l.data.f;
		
		return a.data.v <= u.data.f && a.data.v >= l.data.f;
	}
	if (u.type == INT)
		return a.data.f <= u.data.v && a.data.f >= l.data.f;
		
	return a.data.f <= u.data.f && a.data.f >= l.data.f;
}

// Increment a value
void l_inc_v(VAL* it, VAL inc)
{
	if (!((it->type == INT || it->type == FLOAT) && (inc.type == INT || inc.type == FLOAT)))
	{
		printf(KRED "Error:" KRESET " Non-numeric values used in list contstructor.\n");
		exit(EXIT_FAILURE);
	}
	if (it->type == INT)
	{
		if (inc.type == INT)
		{
			it->data.v += inc.data.v;
			return;
		}
		int v = it->data.v;
		it->type = FLOAT;
		it->data.f = inc.data.f + v;
		return;
	}
	if (inc.type == INT)
	{
		it->data.f += inc.data.v;
		return;
	}
	it->data.f += inc.data.f;
	return;
}
void l_reset()
{
	clear_f_vars();
	stack_len = stackbase[fdepth];
	fdepth--;
}
void l_end()
{
	l_return(nullptr_p);
}
void l_return(int r)
{
	l_reset();
	l_push(r);
}
void l_setbase()
{
	if (stackbase == NULL)
		stackbase = malloc(sizeof(int) * MAX_CALL_DEPTH);
	stackbase[fdepth] = stack_len;
}
void l_gt(int v2p, int v1p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);
	
	VAL out;
	out.type = BOOL;
	
	if (!((v1.type == INT || v1.type == FLOAT) && (v2.type == INT || v2.type == FLOAT)))
	{
		out.data.b = 0;
		l_push(saveToSpace(out));
		printf(KRED "Error:" KRESET " Attempting '>' operation on incompatible types.\n");
		exit(EXIT_FAILURE);
	}

	if (v1.type == INT)
	{
		if (v2.type == INT)
		{
			out.data.b = v1.data.v > v2.data.v;
			l_push(saveToSpace(out));
			return;
		}
		
		out.data.b = v1.data.v > v2.data.f;
		l_push(saveToSpace(out));
		return;
	}
	if (v2.type == INT)
	{
		out.data.b = v1.data.f > v2.data.v;
		l_push(saveToSpace(out));
		return;
	}
	out.data.b = v1.data.f > v2.data.f;
	l_push(saveToSpace(out));
}
void l_onto(int v1p, int v2p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);
	VAL nval;
	nval.type = LIST;

	if (v1.type != LIST)
	{
		printf(KRED "Error:" KRESET " 'onto' operator can only be used with a list.\n");
		exit(EXIT_FAILURE);
	}

	nval.data.l.length = v1.data.l.length + 1;
	nval.data.l.values = malloc (sizeof(int) * nval.data.l.length);
	for (int i = 0; i < v1.data.l.length; i++)
	{
		nval.data.l.values[i] = v1.data.l.values[i];
	}
	nval.data.l.values[v1.data.l.length] = v2p;
	free(v1.data.l.values);
	l_push(updateInSpace(v1p, nval));
}

void l_lt(int v2p, int v1p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);
	
	VAL out;
	out.type = BOOL;
	
	if (!((v1.type == INT || v1.type == FLOAT) && (v2.type == INT || v2.type == FLOAT)))
	{
		out.data.b = 0;
		l_push(saveToSpace(out));
		printf(KRED "Error:" KRESET " Attempting '<' operation on incompatible types.\n");
		exit(EXIT_FAILURE);
	}

	if (v1.type == INT)
	{
		if (v2.type == INT)
		{
			out.data.b = v1.data.v < v2.data.v;
			l_push(saveToSpace(out));
			return;
		}
		
		out.data.b = v1.data.v < v2.data.f;
		l_push(saveToSpace(out));
		return;
	}
	if (v2.type == INT)
	{
		out.data.b = v1.data.f < v2.data.v;
		l_push(saveToSpace(out));
		return;
	}
	out.data.b = v1.data.f < v2.data.f;
	l_push(saveToSpace(out));
	
}
void l_gte(int v2p, int v1p){

	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);
	
	VAL out;
	out.type = BOOL;
	
	if (!((v1.type == INT || v1.type == FLOAT) && (v2.type == INT || v2.type == FLOAT)))
	{
		out.data.b = 0;
		l_push(saveToSpace(out));
		printf(KRED "Error:" KRESET " Attempting '>=' operation on incompatible types.\n");
		exit(EXIT_FAILURE);
	}

	if (v1.type == INT)
	{
		if (v2.type == INT)
		{
			out.data.b = v1.data.v >= v2.data.v;
			l_push(saveToSpace(out));
			return;
		}
		
		out.data.b = v1.data.v >= v2.data.f;
		l_push(saveToSpace(out));
		return;
	}
	if (v2.type == INT)
	{
		out.data.b = v1.data.f >= v2.data.v;
		l_push(saveToSpace(out));
		return;
	}
	out.data.b = v1.data.f >= v2.data.f;
	l_push(saveToSpace(out));
}
void l_lte(int v2p, int v1p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);
	
	VAL out;
	out.type = BOOL;
	
	if (!((v1.type == INT || v1.type == FLOAT) && (v2.type == INT || v2.type == FLOAT)))
	{
		out.data.b = 0;
		l_push(saveToSpace(out));
		printf(KRED "Error:" KRESET " Attempting '<=' operation on incompatible types.\n");
		exit(EXIT_FAILURE);
	}

	if (v1.type == INT)
	{
		if (v2.type == INT)
		{
			out.data.b = v1.data.v <= v2.data.v;
			l_push(saveToSpace(out));
			return;
		}
		
		out.data.b = v1.data.v <= v2.data.f;
		l_push(saveToSpace(out));
		return;
	}
	if (v2.type == INT)
	{
		out.data.b = v1.data.f <= v2.data.v;
		l_push(saveToSpace(out));
		return;
	}
	out.data.b = v1.data.f <= v2.data.f;
	l_push(saveToSpace(out));	
}
void l_mult(int v1p, int v2p)
{
	
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL nval;
	nval.type = UDEF;

	switch (v1.type)
	{
		case INT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = INT;
					nval.data.v = v1.data.v * v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.v * v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		case FLOAT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f * v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f * v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	l_push(saveToSpace(nval));

	if (nval.type == UDEF)
	{
		printf(KRED "Error:" KRESET " '*' operation not valid on non-numeric operands.\n");
		exit(EXIT_FAILURE);
	}
}
void l_div(int v2p, int v1p)
{

	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL nval;
	nval.type = UDEF;

	switch (v1.type)
	{
		case INT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = INT;
					nval.data.v = v1.data.v / v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.v / v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		case FLOAT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f / v2.data.v;
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = v1.data.f / v2.data.f;
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	l_push(saveToSpace(nval));

	if (nval.type == UDEF)
	{
		printf(KRED "Error:" KRESET " '/' operation not valid on non-numeric operands.\n");
		exit(EXIT_FAILURE);
	}	
}
void l_and(int v1p, int v2p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL obj;
	obj.data.b = (v1.type != BOOL || v1.data.b) && (v2.type != BOOL || v2.data.b);
	obj.type = BOOL;
	l_push(saveToSpace(obj));
}
void l_ind(int indp, int listp)
{
	VAL ind = getVal(indp);
	VAL list = getVal(listp);

	if (list.type != LIST)
	{
		printf(KRED "Error:" KRESET " Getting element from non-list expression.\n");
		exit(EXIT_FAILURE);
	}

	if (ind.type != INT)
	{
		printf(KRED "Error:" KRESET " Getting element from list by non-integer index.\n");
		exit(EXIT_FAILURE);
	}

	if (ind.data.v < 0 || ind.data.v >= list.data.l.length)
	{
		printf(KRED "Error:" KRESET " Index out of range.\n");
		exit(EXIT_FAILURE);
	}

	l_push(list.data.l.values[ind.data.v]);

}
void l_pow(int v2p, int v1p)
{
	VAL v1 = getVal(v1p);
	VAL v2 = getVal(v2p);

	VAL nval;
	nval.type = UDEF;

	switch (v1.type)
	{
		case INT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = INT;
					nval.data.v = pow(v1.data.v,v2.data.v);
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = pow(v1.data.v,v2.data.f);
					break;
				}
				default:
					break;
			}
			break;
		}
		case FLOAT:
		{
			switch (v2.type)
			{
				case INT:
				{
					nval.type = FLOAT;
					nval.data.f = pow(v1.data.f,v2.data.v);
					break;
				}
				case FLOAT:
				{
					nval.type = FLOAT;
					nval.data.f = pow(v1.data.f,v2.data.f);
					break;
				}
				default:
					break;
			}
			break;
		}
		default:
			break;
	}

	l_push(saveToSpace(nval));

	if (nval.type == UDEF)
	{
		printf(KRED "Error:" KRESET " '^' operation not valid on non-numeric operands.\n");
		exit(EXIT_FAILURE);
	}
}
void l_squashlist(int size)
{
	VAL out;
	out.type = LIST;
	out.data.l.length = size;
	out.data.l.values = malloc(sizeof(int) * size);

	for (int i = 0; i < size; i++)
	{
		out.data.l.values[size - 1 - i] = l_pop();
	}

	l_push(saveToSpace(out));
}
void l_genlist(int startp, int endp, int stepp)
{
	VAL end = getVal(endp);
	VAL step = getVal(stepp);
	VAL start = getVal(startp);
	float stepval = 0;
	if (step.type == INT) stepval += step.data.v;
	if (step.type == FLOAT) stepval += step.data.f;

	VAL out;
	out.type = LIST;
	int size = floor(fabs(diff(end, start)) / stepval) + 1;
	out.data.l.length = size;
	out.data.l.values = malloc(sizeof(VAL) * size);

	int pos = 0;
	for (VAL i = start; l_check_range_v(start, i, end); l_inc_v(&i, step))
	{
		out.data.l.values[pos++] = saveToSpace(i);
	}

	l_push(saveToSpace(out));
}
void l_updatelist(int indexp, int listp, int newValp)
{
	VAL ind = getVal(indexp);
	VAL list = getVal(listp);

	if (list.type != LIST)
	{
		printf(KRED "Error:" KRESET " Getting element from non-list expression.\n");
		exit(EXIT_FAILURE);
	}

	if (ind.type != INT)
	{
		printf(KRED "Error:" KRESET " Getting element from list by non-integer index.\n");
		exit(EXIT_FAILURE);
	}

	if (ind.data.v < 0 || ind.data.v >= list.data.l.length)
	{
		printf(KRED "Error:" KRESET " Index out of range.\n");
		exit(EXIT_FAILURE);
	}

	list.data.l.values[ind.data.v] = newValp;
	
	l_push(updateInSpace(listp, list));
}
void l_memb(int objp, char* field)
{
	VAL obj = getVal(objp);

	if (obj.type != OBJ)
	{
		printf(KRED "Error:" KRESET " Getting field from non-object expression.\n");
		exit(EXIT_FAILURE);
	}

	char okay = 0;
	int i = 0;
	for (i = 0; i < obj.data.o.length; i++)
	{
		if (strcmp(field, obj.data.o.fields[i]) == 0)
		{
			okay = 1;
			break;
		}
	}

	if (!okay)
	{
		printf(KRED "Error:" KRESET " Object does not contain \"%s\" field.\n", field);
		exit(EXIT_FAILURE);
	}

	l_push(obj.data.o.values[i]);
}

void l_new()
{
	l_push(saveToSpace(emptyObj()));
}

// Create an empty object
VAL emptyObj()
{
	VAL obj;
	obj.type = OBJ;
	obj.data.o.length = 0;
	obj.data.o.values = NULL;
	obj.data.o.fields = NULL;
	return obj;
}


void l_updatefield(int objectp, int valuep, char* field)
{
	VAL object = getVal(objectp);

	if (object.type != OBJ)
	{
		printf(KRED "Error:" KRESET " Getting field from non-object expression.\n");
		exit(EXIT_FAILURE);
	}

	char exists = 0;
	int i = 0;
	for (i = 0; i < object.data.o.length; i++)
	{
		if (strcmp(field, object.data.o.fields[i]) == 0)
		{
			exists = 1;
			break;
		}
	}

	if (exists)
	{
		object.data.o.values[i] = valuep;
		return;
	}

	VAL new_o;
	new_o.type = OBJ;
	new_o.data.o.length = object.data.o.length + 1;
	new_o.data.o.fields = malloc (sizeof (char*) * new_o.data.o.length);
	new_o.data.o.values = malloc (sizeof (int) * new_o.data.o.length);
	for (int i = 0; i < object.data.o.length; i++)
	{
		new_o.data.o.fields[i] = object.data.o.fields[i];
		new_o.data.o.values[i] = object.data.o.values[i];
	}

	free(object.data.o.values);
	free(object.data.o.fields);

	new_o.data.o.fields[object.data.o.length] = malloc (sizeof(char) * (strlen(field) + 1));
	strcpy(new_o.data.o.fields[object.data.o.length], field);
	new_o.data.o.values[object.data.o.length] = valuep;

	l_push(updateInSpace(objectp, new_o));
}

// Turn a value into a string
void sprintVal(char* target, VAL v)
{
	switch (v.type)
	{
		case UDEF:
		{
			sprintf(target, KBLU "Undefined" KRESET);
			break;
		}
		case OBJ:
		{
			sprintf(target, KMAG "{" KRESET);
			target+=strlen(target);
			if (v.data.o.length > 0)
			{
				for(int i = 0; i < v.data.o.length-1; i++)
				{
					sprintf(target, KMAG "%s = " KRESET, v.data.o.fields[i]);
					target+=strlen(target);

					VAL v_ = getVal(v.data.o.values[i]);
					if (v_.type != OBJ && v_.type !=LIST)
					{
						sprintVal(target, v_);
					} else if (v_.type == OBJ) {
						sprintf(target, KMAG "{OBJECT}" KRESET);
					} else {
						sprintf(target, KYEL "[LIST]" KRESET);
					}

					target+=strlen(target);
					sprintf(target, KMAG "," KRESET);
					target+=strlen(target);
				}
				sprintf(target, KMAG "%s = " KRESET, v.data.o.fields[v.data.o.length-1]);
				target+=strlen(target);


				VAL v_ = getVal(v.data.o.values[v.data.o.length-1]);
				if (v_.type != OBJ && v_.type !=LIST)
				{
					sprintVal(target, v_);
				} else if (v_.type == OBJ) {
					sprintf(target, KMAG "{OBJECT}" KRESET);
				} else {
					sprintf(target, KYEL "[LIST]" KRESET);
				}

				target+=strlen(target);
			} else {
				sprintf(target, KMAG "EMPTY OBJECT" KRESET);
				target+=strlen(target);
			}
			sprintf(target, KMAG "}" KRESET);
			break;
		}
		case LIST:
		{
			sprintf(target, KYEL "[" KRESET);
			target+=strlen(target);
			if (v.data.l.length > 0)
			{
				for(int i = 0; i < v.data.l.length-1; i++)
				{

					VAL v_ = getVal(v.data.l.values[i]);
					if (v_.type != OBJ && v_.type !=LIST)
					{
						sprintVal(target, v_);
					} else if (v_.type == OBJ) {
						sprintf(target, KMAG "{OBJECT}" KRESET);
					} else {
						sprintf(target, KYEL "[LIST]" KRESET);
					}

					target+=strlen(target);

					sprintf(target, KYEL "," KRESET);
					target+=strlen(target);
				}
				VAL v_ = getVal(v.data.l.values[v.data.l.length-1]);
				if (v_.type != OBJ && v_.type !=LIST)
				{
					sprintVal(target, v_);
				} else if (v_.type == OBJ) {
					sprintf(target, KMAG "{OBJECT}" KRESET);
				} else {
					sprintf(target, KYEL "[LIST]" KRESET);
				}

				target+=strlen(target);
			}
			sprintf(target, KYEL "]" KRESET);
			break;
		}
		case FID:
		{
			sprintf(target, KBLU "Function" KRESET);
			break;
		}
		case INT:
		{
			sprintf(target, "%d", v.data.v);
			break;
		}
		case BOOL:
		{
			sprintf(target, "%s", v.data.b ? KGRN "true" KRESET : KRED "false" KRESET);
			break;
		}
		case FLOAT:
		{
			sprintf(target, "%.3f", v.data.f);
			break;
		}
		case STR:
		{
			sprintf(target, "%s", v.data.str);
			break;
		}
		default:
		{
			break;
		}
	}
}