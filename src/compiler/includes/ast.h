// Function Call
typedef struct Call {
	struct Expr* lhs;
	int argc;
	struct Expr* args;
} Call;

// Variable
typedef struct Var {
	char* name;
} Var;

// Operators
typedef enum {
	IND,
	POW,
	MULT,
	DIV,
	MINUS,
	PLUS,
	NOT,
	ONTO,
	GT,
	LT,
	GTE,
	LTE,
	IS,
	OR,
	AND,
	MEMB
} Operator;

// Control Type
typedef enum {
	BREAK,
	CONTINUE
} ControlType;

// Control
typedef struct Control {
	ControlType type;
} Control;

// Operation
typedef struct Op {
	Operator type;
	struct Expr* a;
	struct Expr* b;
} Op;

// Constant Type
typedef enum {
	UNDEF,
	INT,
	OBJ,
	STR,
	FLOAT,
	BOOL,
	LIST,
	LISTCON,
	FID
} CType;

// List
typedef struct List {
	int length;
	struct Expr* values;
} List;

// List Constructor
typedef struct ListCon {
	struct Expr* start;
	struct Expr* end;
	struct Expr* step;
} ListCon;

// Constant
typedef struct Const {
	CType type;
	union {
		int v;
		char* str;
		//Don't store as a float here
		char* f;
		char b;
		List l;
		ListCon lc;
		int fid;
	} data;
} Const;

// Expression Type
typedef enum {
	NONE,
	CONST,
	VAR,
	OP,
	CALL
} EType;

// Expression
typedef struct Expr {
	EType type;
	union {
		Const* conste;
		Var* vare;
		Call* calle;
		Op* ope;
	} data;
} Expr;

// If Statement
typedef struct If {
	Expr* expression;
	int ificount;
	int elicount;
	struct Instr* ifinstrs;
	struct Instr* elseinstrs;
} If;

// For Statement
typedef struct For {
	Expr* iterator;
	Expr* expression;
	int icount;
	struct Instr* instrs;
} For;

// While Statement
typedef struct While {
	Expr* expression;
	int icount;
	struct Instr* instrs;
} While;

// Assign Statement
typedef struct Assign {
	Expr* lhs;
	Expr* rhs;
} Assign;

// Print Statement
typedef struct Print {
	Expr* expression;
} Print;

// Return Statement
typedef struct Return {
	Expr* expression;
} Return;

// Instruction Type
typedef enum {
	EMPTY,
	PRINT,
	ASSIGN,
	WHILE,
	FOR,
	IF,
	CONTROL,
	RETURN
} IType;

// Instructions
typedef struct Instr {
	IType type;
	union data {
		Print* printi;
		Assign* assigni;
		While* whilei;
		If* ifi;
		Control* controli;
		Return* returni;
		For* fori;
	} data;
	char* fname;
	int lineNo;
	char* orig;
} Instr;

// Function Type
typedef enum {
	NORMAL,
	MAIN,
	METHOD
} FType;

// Function
typedef struct Func {
	FType type;
	int fid;
	int icount;
	int acount;
	char** args;
	Instr* instrs;
} Func;

// Abstract Syntax Tree
typedef struct AST {
	int funcc; // Function Count
	Func* functions;
} AST;