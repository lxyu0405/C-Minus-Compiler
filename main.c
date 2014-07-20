#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header/treenode.h"
#include "header/symbol.h"
#include "header/intercode.h"
#include "semantic/hash.c"
#include "semantic/semantic.c"
#include "intercode/intercode.c"
#include "mipscode/mipscode.c"

/*#define YYDEBUG 1*/
/*extern FILE* yyin;*/
/*extern int yydebug;*/
int g_error = 0;
treenode *g_treeroot = NULL;
int main(int argc, char** argv)
{
	initHashList();
	if(argc <= 1)
		return 1;
	FILE* f = fopen(argv[1],"r");
	if(!f)
	{
		perror(argv[1]);
		return 1;
	}
	yyrestart(f);

	FILE* file = fopen(argv[2],"w");
	if(!file || strcmp(argv[2],"") == 0){
		printf("open write file error, please confirm the file name\n");
		printf("*** Format: './parser [source file] [target file]'\n ***");
		return 1;
	}
//	yydebug = 1;
	yyparse();
	if(g_error == 0)
	{
		if(g_treeroot == NULL)
			printf("Error: tree_root is NULL\n");
		else
		{
			//printTree(g_treeroot,0);
			init();
			analyze(g_treeroot);
			//printHashList(argList,"argList");
			//printHashList(funcList,"funcList");
			//printHashList(structList,"structList");
			InterCodeNode intercode = translate_Program(g_treeroot);
			optimize(intercode);
			//show(intercode);
			//writeFile(intercode,file);
			generateMIPS(intercode,file);
		}
	}
	//checkExtern();
	//printf("************Symbol List***********\n");
	return 0;
}


