#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//Parameters
#define MAX_STACK_SIZE 1024*8
#define MAX_VAR_COUNT 1024*8
#define MAX_STRING_LENGTH 1024*1024*8
#define MAX_CALL_DEPTH 1024*32
#define MAX_SPACE 1024*1024

//Console Colours
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KRESET "\033[0m"

//Call a function by fid
int runFunc(int fid, int obj);
int getArgC(int fid);

typedef enum {
	UDEF,
	INT,
	STR,
	FLOAT,
	BOOL,
	LIST,
	OBJ,
	FID
} Type;

// List Data Type
typedef struct List {
	int length;
	int* values;
} List;

// Object Data Type
typedef struct Obj {
	int length;
	char** fields;
	int* values;
} Obj;

// Value Data Type
typedef struct VAL {
	Type type;
	union {
		int v;
		char* str;
		float f;
		char b;
		List l;
		Obj o;
		int fid;
	} data;
} VAL;

// Operations
VAL getVal(int ptr);

int l_pop();
void l_push(int ptr);
void l_print(int v);
void l_conststr(char* str);
void l_constfid(int fid);
void l_constint(int v);
void l_constfloat(float f);
void l_constbool(char b);
void l_store(int p, char* name);
void l_load(char* name);
void l_call(int o, int v, int argc);
void l_not(int v);
char l_if(int v);
void l_plus(int v1, int v2);
void l_minus(int v1, int v2);
void l_is(int v1, int v2);
void l_or(int v1, int v2);
char l_check_range(int l, int a, int u);
void l_inc(int it, int inc);
void l_reset();
void l_end();
void l_return(int r);
void l_setbase();
void l_gt(int v1, int v2);
void l_onto(int v1, int v2);
void l_lt(int v1, int v2);
void l_gte(int v1, int v2);
void l_lte(int v1, int v2);
void l_mult(int v1, int v2);
void l_div(int v1, int v2);
void l_and(int v1, int v2);
void l_ind(int ind, int list);
void l_pow(int v1, int v2);
void l_squashlist(int size);
void l_genlist(int start, int end, int step);
void l_updatelist(int index, int list, int newVal);
void l_memb(int obj, char* field);
void l_new();
void l_updatefield(int object, int value, char* field);