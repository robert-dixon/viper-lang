/*
	Viper Compiler
	Robert Dixon 2015
*/
#include "includes/include.h"

char* cmd; // Command text for system command
char oname_specified = 0; // Is the output path specified by user
char *fname = NULL; // Source path
char *oname = NULL; // Output path
AST* ast = NULL; // Generated AST
BC* bc = NULL; // Generated Bytecode
char* cCode = NULL; // Generated C Code

void cleanUp(); // Clean up memory

// Main function
int main(int argc, char** args)
{

	// Check environment variable
	if (getenv("VPRDIR") == NULL)
	{
		printf(KRED "Error" KRESET ": There is a problem with the viper installation.  Please re-install.\n");
		return EXIT_FAILURE;
	}

	// Get command-line options
	char showAST = 0;
	char showBC = 0;
	char showC = 0;

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(args[i], "--showAST") == 0)
		{
			showAST = 1;
			continue;
		}
		if (strcmp(args[i], "--showBC") == 0)
		{
			showBC = 1;
			continue;
		}
		if (strcmp(args[i], "--showC") == 0)
		{
			showC = 1;
			continue;
		}
		if (strcmp(args[i], "-o") == 0 && i != argc - 1)
		{
			i++;
			oname = args[i];
			oname_specified = 1;
			continue;
		}
		if (fname == NULL)
		{
			fname = args[i];
		}
	}

	// Check for an input argument - the source file
	if (fname == NULL)
	{
		printf(KRED "Error" KRESET ": Please specify a viper source file.\n");
		return EXIT_FAILURE;
	}

	// Open the file
	FILE *file;
	file = fopen(fname, "r");

	// Failed to open source file
	if (!file)
	{
		printf(KRED "Error" KRESET ": The source file \"%s\" cannot be opened.\n", fname);
		return EXIT_FAILURE;		
	}

	// Decide on output path if it is not specified
	if (oname == NULL)
	{
		oname = malloc((strlen(fname) + 5) * sizeof(char));
		sprintf(oname,"%s.out",fname);
	}

	// Get just file name, not path - for debug information
	char* path = fname;
	while (strchr(fname, '/') != NULL)
		fname = strchr(fname, '/') + 1;

	// Parse the viper source
	ast = loadToAST(file, fname, path, NULL, 0, NULL, 0);

	// Exit on failure
	if (ast == NULL || !typeCheck(ast))
	{
		cleanUp();
		return EXIT_FAILURE;
	}
	// Display the AST if appropriate
	if (showAST) dumpAST(ast);
	
	// Compile to Bytecode
	bc = compileProgram(ast);
	// Display Bytecode if appropriate
	if (showBC) dumpBC(bc);

	// Convert Bytecode to C output
	cCode = cGen(bc);
	// Display if appropriate
	if (showC) printf("%s\n", cCode);

	// Create output folder with system commands
	// And copy the compile files to that directory
	cmd = malloc(4096);
	sprintf(cmd, "mkdir %s_build > /dev/null", oname);
	system(cmd);
	sprintf(cmd, "cp $VPRDIR/compdir/lib.h %s_build > /dev/null", oname);
	system(cmd);
	sprintf(cmd, "cp $VPRDIR/compdir/main.c %s_build > /dev/null", oname);
	system(cmd);
	sprintf(cmd, "cp $VPRDIR/compdir/makefile %s_build > /dev/null", oname);
	system(cmd);

	// Write the C output to file
	sprintf(cmd, "%s_build/out.c", oname);
	FILE *outfile = fopen(cmd, "wb");
	// Failed to open target file
	if (!outfile)
	{
		sprintf(cmd, "rm -r %s_build > /dev/null", oname);
		system(cmd);
		printf(KRED "Error" KRESET ": The output cannot be saved.\n");
		cleanUp();
		return EXIT_FAILURE;		
	}
	fputs(cCode, outfile);
	fclose(outfile);

	// Build the program and copy the output to the output path
	sprintf(cmd, "cd %s_build\n make > /dev/null", oname);
	system(cmd);
	sprintf(cmd, "mv %s_build/build %s > /dev/null", oname, oname);
	system(cmd);

	// Tidy memory
	cleanUp();

	// End
	printf("Successfully Compiled!\n");
	return EXIT_SUCCESS;
}

// Track extra bits of memory allocated
// That need keeping, so we can clear them
// Out at the end
void** ex_mem;
int ex_mem_count = 0;
void* extraMemory(int size)
{
	void* mem = malloc(size);

	if (ex_mem == NULL)
		ex_mem = malloc(sizeof(void*) * MAX_EXTRA_MEM_ITEMS);

	ex_mem[ex_mem_count++] = mem;
	return mem;
}

// Clear up the extra Memory
void freeExtraMemory()
{
	for (int i = 0; i < ex_mem_count; i++)
		free(ex_mem[i]);
}

// Clean up memory
void cleanUp()
{
	// Remove the output folder
	sprintf(cmd, "rm -r %s_build > /dev/null", oname);
	system(cmd);

	// Free memory
	freeExtraMemory();
	if (cmd != NULL) free(cmd);
	if (cCode != NULL) free(cCode);
	if (!oname_specified) free(oname);
	if (ast != NULL) freeAST(ast);
	if (bc != NULL) freeBC(bc);
}