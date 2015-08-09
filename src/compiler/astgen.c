#include "includes/include.h"

// Different types of code block:
// 		Normal - takes a block of code
//			(eg WHILE, FOR, etc)
// 		Else - a secondary block for an instruction that takes two blocks
//			(only used for IF)
//		Function - body of a function definition
typedef enum {
	BNORMAL,
	BELSE,
	BFUNC,
	BMETHOD
} BlockMode;

// Compress the previous few instructions into a new code block
// And affix it to the relevant instruction create a new function
void wrapBlock(int loc, int last,
				Instr** instrList, int* iPos,
				BlockMode mode,	int* fcount,
				Func** fns, char** aList,
				int acount, int* fdepth);

// Parse the source code to an AST
AST* loadToAST(FILE* source, char*fname, char* path,
				char** importHistory, int importHistoryCount,
				char** libs, int libCount) {

	// Flag to determine whether it has failed
	char failAST = 0;

	// Define main function for the program
	Func mainFunction;
	mainFunction.type = MAIN;
	mainFunction.fid = 0;
	mainFunction.acount = 0;
	mainFunction.args = NULL;
	int fncount = 0;
	int fdepth = 0;

	// Define list of other functions
	Func* fns = malloc(MAX_FUNCTION_COUNT * sizeof(Func));

	// List of instructions in the source file
	Instr** instrList = malloc(MAX_FUNCTION_LENGTH * sizeof(Instr*));
	int instrCount = 0;

	// Buffer for reading an instruction
	char* instrBuffer = malloc(MAX_INSTRUCTION_LENGTH * sizeof(char));
	int iPos = 0;

	// Flags for interpreting instructions
	char ignoreLineBreak = 0; // Toggled when line ends with '\'
	char insideString = 0; // Are we currently reading inside a double-quoted string
	char skipChar = 0; // Should we skip past the next char
	char escape = 0; //Is the string currently in escape mode
	char blockStarting = 1;	// Is the next instruction the first in a block?
	char lineStart = 0; // Are we at the start of a line (ie having to skip past indendation)
	char getIndent = 1; // Are we inspecting the indentation of the following line?
	char commentMode = 0; // Are we currently reading inside a comment

	// Indentation at the start of the line
	int spaceIndent = 0;
	int tabIndent = 0;


	// Store the original source code of the line to output
	// To the user for error information
	char* orig = NULL;

	// Stores the indentation levels for each level
	// of nested block in a stack
	int blockIndent[MAX_INDENT_LEVEL];
	// Stores the first instruction in a block for
	// each level of nexted block in a stack
	int blockStart[MAX_INDENT_LEVEL];
	// Stores what kind of block each block is
	// in a stack
	BlockMode mode[MAX_BLOCK_DEPTH];

	int lvl = -1; // Current block level

	// A list of arguments to associate with function definitions
	char*** argList = malloc(sizeof(char***) * MAX_BLOCK_DEPTH);
	int* aCount = malloc(sizeof(int) * MAX_BLOCK_DEPTH);

	// The current character
	char chr;

	// Read through source file
	int lineNo = 0;
	while (1) {

		chr = fgetc(source);

		// Record an accurate source line number
		// For debug information
		if (chr == '\n' || chr == EOF) lineNo++;

		// If it's in a comment, ignore everything
		// until the end of the comment
		if (commentMode)
		{
			if (chr == '\\' && !escape) escape = 1;
			if (chr == '\'' && !escape)	commentMode = 0;

			escape = 0;
			continue; // Next character
		}
	
		// End of an instruction - new line character
		if ((chr == '\n' && !ignoreLineBreak) || chr == EOF)
		{

			lineStart = 0; // The next line has not started
			getIndent = 1; // Inspect indendation of the following line
			instrBuffer[iPos] = 0; // End the string for the instruction
			iPos = 0; // Reset position counter
			// Get the original source text for the line
			orig = realloc(orig, (strlen(instrBuffer)+1) * sizeof(char));
			strcpy(orig, instrBuffer);

			// Calculate indendation level
			int thisIndent = (tabIndent*4) + spaceIndent;

			// Ignore empty instructions
			if (strlen(orig) > 0)
			{
				// If the instruction is the first in a block
				if (blockStarting)
				{
					// Ensure the indentation is one level further than the
					// Parent block, or provide a warning
					if (!(lvl == -1 || thisIndent == blockIndent[lvl] + 4 || thisIndent <= blockIndent[lvl]))
					{
						printf(KYEL "Warning" KRESET " in " KYEL "%s" KRESET ": ine " KYEL "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\t%s\n",
						fname, lineNo, orig, "The indentation of this line is incorrect.");
					}
					// Record the new block level
					lvl++;
					blockStart[lvl] = instrCount;
					blockIndent[lvl] = (lvl == 0 ) ? thisIndent : (blockIndent[lvl-1] + 4);
				}

				// If the indentation is less than the current block
				// Then close off the current block and return to the parent
				// Until the indentation levels match again
				while (thisIndent < blockIndent[lvl])
				{
					lvl--;

					// If we end up going higher-level than the outermost block
					// Then we've messed up
					if (lvl < 0) {
						blockIndent[0] = thisIndent;
						lvl = 0;
						printf(KYEL "Warning" KRESET " in " KYEL "%s" KRESET ": ine " KYEL "%d"
						KRESET":\n\t" KCYN "%s" KRESET "\n\t%s\n",
						fname, lineNo, orig, "The indentation of this line is incorrect.");
					} else {
						// Compress all the instructions since the start of the current block
						// Into a block and attribute it to the correct instruction
						// Or create a function for it, as applicable
						wrapBlock(blockStart[lvl+1]-1, instrCount-1,
									instrList, &instrCount,
									mode[lvl], &fncount,
									&fns, argList[fdepth-1],
									aCount[fdepth-1], &fdepth);
					}
				}

				// If we've just changed the indendation without a reason, throw a warning
				if (blockIndent[lvl] != thisIndent) {
					blockIndent[lvl] = thisIndent;
					printf(KYEL "Warning" KRESET " in " KYEL "%s" KRESET ": ine " KYEL "%d"
					KRESET":\n\t" KCYN "%s" KRESET "\n\t%s\n",
					fname, lineNo, orig, "The indentation of this line is incorrect.");
				}

				// Error message for parsing this instruction
				char* error = NULL;

				// Reset the blockStarting flag
				blockStarting = 0;
				char takesBlock;

				// If the instruction is an imoport statement
				// Then, it needs to be dealt with here
				if (strstr(instrBuffer, "import ") == instrBuffer)
				{

					// Import statements load other viper files
					// into the same AST
					char* fname = instrBuffer; // Get the file name

					fname += 7; // Skip past the import keyword
					//Skip past any whitespace at the start of the path
					while (*fname == ' ' || *fname ==  '\t') fname++;
					// Remove the whitespace from the end of the path
					for (int i = strlen(fname) - 1; i >= 0; i--)
					{
						if (fname[i] != ' ' && fname[i] != '\t')
						{
							fname[i+1] = 0;
							break;
						}
					}

					
					char* pathAbs; // Stores the absolute file path

					char noImport = 0; // Are we ignoring the import?

					// If there's no extension, and no directory path
					// Then the import is a package
					if (strchr(fname, '/') == NULL && strchr(fname, '.') == NULL)
					{
						// Search through the imported libraries to check if this
						// Has already been imported
						for (int i = 0; i < libCount; i++)
						{
							if (strcmp(libs[i], fname) == 0)
							{
								// Do not import library files multiple times
								noImport = 1;
								break;
							}
						}

						// Is it a new libarary?
						if (!noImport)
						{
							// Extend the list of libraries to include this one
							char** newLibs = malloc (sizeof(char*) * (libCount + 1));
							for (int i = 0; i < libCount; i++) newLibs[i] = libs[i];
							newLibs[libCount] = malloc(strlen(fname) * sizeof(char));
							strcpy (newLibs[libCount], fname);
							libs = newLibs;
							libCount++;

							// Get the absolute path of the library code
							char* dir = getenv("VPRDIR");
							pathAbs = malloc((strlen(dir) + strlen("/packages/") + strlen(fname) + 1) * sizeof(char));
							sprintf(pathAbs, "%s/packages/%s/main.vpr", dir, fname);
						}
					}

					// The import is not a package, it's another viper file
					else
					{
						// Get the absolute path of the imported file
						pathAbs = malloc((strlen(path) + strlen(fname) + 1) * sizeof(char));

						// If it starts with a '/' then it's alread an absolute path
						if (strchr(fname, '/') == fname)
						{
							strcpy(pathAbs, fname);
						}
						// Otherwise, get the path relative to the current file
						else
						{
							// Plug the current file path into the absolute path
							strcpy(pathAbs, path);
							// Put the new file path after the last / of the current file path
							char* s = pathAbs;
							while (strchr(s, '/') != NULL) s = strchr(s, '/') + 1;
							strcpy(s, fname);
						}
					}

					// If we're not ignoring the import
					if (!noImport)
					{
						// Strip out and ../ and ./ syntax from the path
						while (strstr(pathAbs, "../") != NULL)
						{		
							char*s = pathAbs;
							while (strstr(s, "../") != (strchr(s, '/') - 2))
							{
								s = strchr(s, '/') + 1;
							}
							char* e = strstr(s, "../") + 3;
							char* tmp = malloc(sizeof(char) * (strlen(e) + 1));
							strcpy(tmp, e);
							strcpy(s, tmp);
							free(tmp);
						}

						while (strstr(pathAbs, "./") != NULL)
						{
							char* tmp = malloc(sizeof(char) * (strlen(pathAbs) + 1));
							strcpy(tmp, pathAbs);
							char* s = strstr(pathAbs, "./");
							char* t = strstr(tmp, "./");
							s += 2;
							strcpy(t, s);
							strcpy(pathAbs, tmp);
							free(tmp);
						}
			
						// Check that the file as not already been imported
						// (circular reference)
						char okay = 1;
						for (int i = 0; i < importHistoryCount; i++)
						{
							if (strcmp(importHistory[i], pathAbs) == 0)
							{
								okay = 0;
								break;
							}
						}

						// Given no circular references
						if (okay)
						{
							// Add this file to the import history for the next import
							char** pathHist = malloc(sizeof(char*) * (importHistoryCount + 1));
							memcpy(pathHist, importHistory, importHistoryCount * sizeof(char*));
							pathHist[importHistoryCount] = path;

							// Get just the file name, without the path
							// For debug information
							char* newFName = pathAbs;
							while (strchr(newFName, '/') != NULL)
								newFName = strchr(newFName, '/') + 1;

							// Open the file
							FILE *file;
							file = fopen(pathAbs, "r");

							// Failed to open source file
							if (!file)
							{
								printf(KYEL "Warning" KRESET ": The source file \"%s\" cannot be opened - import ignored.\n", pathAbs);	
							}
							else
							{
								// Load an AST for the imported source file
								AST* loadFile = loadToAST(file, newFName, pathAbs,
											pathHist, importHistoryCount+1, libs, libCount);
								
								// If it failed, fail this one too.
								if (loadFile == NULL)
								{
									failAST = 1;
								}
								// Otherwise, copy the instructions of that AST into this instruction list
								// And merge the function list with this function list
								else
								{
									for (int i = 0; i < loadFile->functions[0].icount; i++)
									{
										instrList[instrCount] = realloc(instrList[instrCount], sizeof(Instr));
										memcpy(instrList[instrCount], loadFile->functions[0].instrs + i, sizeof(Instr));
										instrCount++;
									}

									for (int i = 1; i < loadFile->funcc; i++)
										fns[fncount++] = loadFile->functions[i];
								}
								free(loadFile->functions);
								free(loadFile);
							}
							free(pathHist);
						}
						else
						{
							// Print a warning that the import was ignored due to a circular dependency
							printf(KYEL "Warning" KRESET ": Circular import dependency - import not completed.\n");	
						}
						free(pathAbs);
					}

					// Import statements do not take a block
					takesBlock = 0;
				}

				// The instruction is not an inport statement, and so should be parsed
				else
				{
					// Parse the instruction and add it to the list of instructions
					instrList[instrCount] = readInstr(instrBuffer, &takesBlock, &error, argList + fdepth, aCount + fdepth);
					// Copy the original line, line number and file name into the instruction
					// structure for debugging information in the type checking phase
					instrList[instrCount]->orig = malloc((strlen(orig) + 1) * sizeof(char));
					strcpy(instrList[instrCount]->orig, orig);
					instrList[instrCount]->fname = malloc((strlen(fname) + 1) * sizeof(char));
					strcpy(instrList[instrCount]->fname, fname);
					instrList[instrCount]->lineNo = lineNo;

					instrCount++;

					// If the instruction was parsed to the empty instruction
					// remove it from the list
					if (instrList[instrCount-1]->type == EMPTY)
					{
						instrCount--;
						free(instrList[instrCount]);
						instrList[instrCount] = NULL;
					}

					// If there was an error parsing the instruction
					if (error != NULL) {
						// Display the error returned in parsing it
						printf(KRED "Error" KRESET " in " KRED "%s" KRESET ": line " KRED "%d"
							KRESET":\n\t" KCYN "%s" KRESET "\n\t%s\n",
							fname, lineNo, orig, error);

						failAST = 1;
					} else {
						// Otherwise, the instruction parsed correctly
						
						// If the instruction is associated with a block
						// Then we need to know what kind of block it will take
						
						// Normal blocks
						if (takesBlock == 1) {
							mode[lvl] = BNORMAL;
							blockStarting = 1;
						}

						// Else blocks
						if (takesBlock == 2) {
							mode[lvl] = BELSE;
							blockStarting = 1;
						}

						// Function Definitions
						if (takesBlock == 3) {
							mode[lvl] = BFUNC;
							blockStarting = 1;
							fdepth++;
						}

						// Method Definitions
						if (takesBlock == 4) {
							mode[lvl] = BMETHOD;
							blockStarting = 1;
							fdepth++;
						}
					}
				}

			}

			// Reset the indendation levels for the next line
			tabIndent = 0;
			spaceIndent = 0;

			// Stop parsing at the end of the file
			if (chr == EOF) break;
			continue;
		}

		// skip the whitespace at the start of a line
		if (lineStart == 0 && (chr == '\t' || chr == ' '))
		{
			skipChar = 1;
			// increase the line indentation
			if (getIndent)
			{
				if (chr == '\t')
				{
					tabIndent++;
				} else {
					spaceIndent++;
				}
			}
		}

		// ignore comments
		if (chr == '\'' && !insideString) {
			skipChar = 1;
			commentMode = 1;
		}

		// ignore line breaks that are preceeded by '\'
		if (chr == '\n' && ignoreLineBreak) {
			skipChar = 1;
			lineStart = 0;
		}

		// ignore following line break
		if (chr == '\\' && !insideString) {
			ignoreLineBreak = 1;
			skipChar = 1;
		}

		if (chr == '"' && !escape) {
			insideString = !insideString;
		}

		if (chr == '\\' && insideString) {
			escape = 1;
		} else if (insideString) {
			escape = 0;
		}

		// Add to the instruction buffer
		if (!skipChar) {
			ignoreLineBreak = 0;
			instrBuffer[iPos++] = chr;
			lineStart = 1;
			getIndent = 0;
		}

		// Reset the skipChar flag
		skipChar = 0;
	}

	AST* out;

	if (!failAST)
	{
		// If finish the file with blocks still open, close them all off
		// Until we get to the outermost level
		while (lvl>0)
		{
			lvl--;
			wrapBlock(blockStart[lvl+1]-1, instrCount-1, instrList, &instrCount,
				mode[lvl], &fncount, &fns, argList[fdepth-1], aCount[fdepth-1], &fdepth);
			
		}

		// Copy the remaining instructions into the main function
		mainFunction.icount = instrCount;
		mainFunction.instrs = malloc(instrCount * sizeof(Instr));
		for (int i = 0; i < instrCount; i++)
		{
			memcpy(mainFunction.instrs+i, instrList[i], sizeof(Instr));
		}

		// Create the AST and add the main function and all
		// the other functions
		out = malloc(sizeof(AST));

		out->funcc = 1 + fncount;
		out->functions = malloc ((fncount + 1) * sizeof(Func));
		out->functions[0] = mainFunction;
		for (int i = 1; i <= fncount; i++)
			out->functions[i] = fns[i-1];
	}
	else
	{
		out = NULL;
	}

	// Tidy Up
	for (int i = 0; i < instrCount; i++)
		free(instrList[i]);
	free (instrList);
	free(instrBuffer);
	for (int i = 0; i < fncount; i++)
		free (argList[i]);
	free(fns);
	free (aCount);
	free (argList);
	return out;
}

// Take all of the last instructions in the list that need to be contained
// In the current block and associate them with the relevant
// Instruction (directly preceeding the block) or make a function
// out of them, depending on the block type
void wrapBlock(int loc, int last, Instr** instrList, int* iPos,
				BlockMode mode, int* fcount, Func** fns,
				char** aList, int acount, int* fdepth)
{
	Instr* base = instrList[loc]; // Instruction associated with block
	int c = last - loc; // Number of instructions in the block
	switch (base->type)
	{
		// Assign only takes a block for function declarations
		case ASSIGN:
		{
			(*fdepth)--; // Move out of the function level to the parent level
			Assign* stat = base->data.assigni;

			// Create the new function
			Func newFunction;
			if (mode == BFUNC)
			{
				newFunction.type = NORMAL;
			} else {
				newFunction.type = METHOD;
			}
			// Get the FID from the assign statement
			newFunction.fid = stat->rhs->data.conste->data.fid;

			// Add the instructions in this block into the new function
			newFunction.icount = c;
			newFunction.acount = acount;
			newFunction.args = malloc(sizeof(char*) * acount);
			memcpy(newFunction.args, aList, sizeof(char*) * acount);
			newFunction.instrs = malloc(c * sizeof(Instr));
			for (int i = 0; i < c; i++)
			{
				memcpy(newFunction.instrs+i,instrList[loc + 1 + i],sizeof(Instr));
				(*iPos) --;
			}

			// Add the function to the function list
			(*fns)[(*fcount)++] = newFunction;
			break;
		}

		// For statements only take one block
		case FOR:
		{
			// Add the instructions from this block
			// into the For instruction
			For* stat = base->data.fori;
			stat->icount = c;

			stat->instrs = malloc(c * sizeof(Instr));
			for (int i = 0; i < c; i++)
			{
				stat->instrs[i] = *(instrList[loc + 1 + i]);
				instrList[loc + 1 + i] = NULL;
				(*iPos) --;
			}
			break;
		}

		// If statements take two blocks
		case IF:
		{
			If* stat = base->data.ifi;

			// Add the instructions from this block
			// into the else part of the If statement
			// if the block mode is else
			// otherwise, into the if part of the If
			// statement

			if (mode == BELSE)
			{
				stat->elicount = c;

				stat->elseinstrs = malloc(c * sizeof(Instr));
				for (int i = 0; i < c; i++)
				{
					stat->elseinstrs[i] = *(instrList[loc + 1 + i]);
					instrList[loc + 1 + i] = NULL;
					(*iPos) --;
				}
			} else {
				stat->ificount = c;

				stat->ifinstrs = malloc(c * sizeof(Instr));
				for (int i = 0; i < c; i++)
				{
					stat->ifinstrs[i] = *(instrList[loc + 1 + i]);
					instrList[loc + 1 + i] = NULL;
					(*iPos) --;
				}
			}
			break;
		}

		// While statements only take one block
		case WHILE:
		{
			// Add the instructions from this block
			// into the While instruction
			While* stat = base->data.whilei;
			stat->icount = c;

			stat->instrs = malloc(c * sizeof(Instr));
			for (int i = 0; i < c; i++)
			{
				stat->instrs[i] = *(instrList[loc + 1 + i]);
				instrList[loc + 1 + i] = NULL;
				(*iPos) --;
			}
			break;
		}
		default:
			break;
	}
	return;
}

// Clear up memory
void freeAST(AST* ast)
{
	for (int i = 0; i < ast->funcc; i++)
	{
		Func fn = ast->functions[i];
		for (int j = 0; j < fn.acount; j++)
		{
			free(fn.args[j]);
		}
		for (int j = 0; j < fn.icount; j++)
		{
			Instr instr = fn.instrs[j];
			freeInstr(instr);
		}
		if (fn.instrs != NULL) free (fn.instrs);
		if (fn.args != NULL) free (fn.args);
	}
	free (ast->functions);
	free (ast);
}