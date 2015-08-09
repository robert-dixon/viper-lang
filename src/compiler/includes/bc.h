// Constant Type
typedef enum {
	BPRINT,
	BCONSTFID,
	BCONSTINT,
	BCONSTSTR,
	BCONSTFLOAT,
	BCONSTBOOL,
	BCONSTOBJ,
	BSTORE,
	BLOAD,
	BIND,
	BPOW,
	BMULT,
	BDIV,
	BMINUS,
	BPLUS,
	BNOT,
	BONTO,
	BGT,
	BLT,
	BGTE,
	BLTE,
	BIS,
	BOR,
	BAND,
	BMEMB,
	BRETURN,
	BCALL,
	BCALLM,
	BSQUASHLIST,
	BGENLIST,
	BIF,
	BYELSE,
	BWHILE,
	BFOREACH,
	BFOR,
	BBREAK,
	BCONTINUE,
	BSETBASE,
	BUPDATELIST,
	BUPDATEFIELD,
	BTHISOBJ,
	BPOP
} BInstrType;

// Instruction Bytecode
typedef struct BInstr {
	BInstrType type;
	int argc;
	char** args;
	int ic;
	struct BInstr* is;
} BInstr;

// Function Bytecode
typedef struct BFunc {
	int instrc;
	int fid;
	int acount;
	BInstr* instructions;
} BFunc;

// Program Bytecode
typedef struct BC {
	int funcc;
	BFunc* functions;
} BC;