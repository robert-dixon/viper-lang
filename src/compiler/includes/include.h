#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Constants
#define MAX_INSTRUCTION_LENGTH 8192
#define MAX_FUNCTION_LENGTH 4096
#define MAX_ARG_COUNT 64
#define MAX_INDENT_LEVEL 64
#define MAX_EXPLICIT_LIST_LEN 256
#define MAX_BLOCK_DEPTH 64
#define MAX_FUNCTION_COUNT 1024
#define MAX_C_PROG_LENGTH 1024*1024
#define MAX_EXTRA_MEM_ITEMS 4096

// Terminal Colours
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define KRESET "\033[0m"

// Bytecode and AST structures
#include "ast.h"
#include "bc.h"

// Read to AST
AST* loadToAST(FILE* source, char* fname, char* path, char** importHistory, int importHistoryCount, char** libs, int libC);
Instr* readInstr(char* line, char* takesBlock, char** error, char*** arglist, int* aCount);
Expr* readExpr(char* expr);

//Compile Program to Bytecode
BC* compileProgram(AST* prog);
//Convert Bytecode to C
char* cGen(BC* bc);

char typeCheck(AST* ast);

// Helpful Functions
char* stripSpace(char* expr);

// Debug Functions
void dumpAST(AST* ast);
void dumpFunc(Func function);
void dumpInstr(Instr instruction);
void dumpExpr(Expr expression);
void dumpConst(Const constant);
void dumpOp(Op operation);
void dumpVar(Var variable);
void dumpCall(Call funccall);
void dumpAssign(Assign assigninstr);
void dumpPrint(Print printinstr);
void dumpIf(If ifinstr);
void dumpWhile(While whileinstr);
void dumpReturn(Return retinstr);
void dumpControl(Control ctrlinstr);
void dumpFor(For forinstr);

void dumpBC(BC* bc);

char* strIType(IType t);
char* strEType(EType t);
char* strCType(CType t);
char* strOType(Operator t);

// Cleanup Functions
void freeAST(AST* ast);
void freeInstr(Instr instr);
void freeExpr(Expr* expr);
void freeBC(BC* bc);

void* extraMemory(int size);
void freeExtraMemory();