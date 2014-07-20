%{
	#include <stdio.h>
	#include <stdlib.h>
	#include "lex.yy.c"
	#include <stdarg.h>
	#include "header/treenode.h"
	#include <string.h>

	treenode* reduce(char *name,int num,...);
	void printTree(treenode *root,int layer);
	void returnspace(treenode *root);
%}

/*declated types*/
%union{
	struct Treenode* type_node;
}

/*declared tokens */
%token <type_node> INT
%token <type_node> FLOAT 
%token <type_node> ID SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT
%token <type_node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE TYPE error

/*declared non-terminals*/
%type <type_node> ExtDefList ExtDef ExtDecList Specifier StructSpecifier
%type <type_node> OptTag Tag VarDec FunDec VarList ParamDec CompSt StmtList
%type <type_node> Program Stmt DefList Def DecList Dec Exp Args


%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%right ASSIGNOP
%left AND OR
%left RELOP
%left PLUS MINUS
%left STAR DIV

%right NOT
%left LP RP
%left LB RB
%left DOT
%%

/*High-Level Definitions*/
Program		:	ExtDefList		{
									$$ = reduce("Program",1,$1);
									//printf("construct tree done!\n");
									//printf("start to print tree\n");
									//if(error == 0)
										g_treeroot = $$;
										//printTree($$,0);
									//returnspace($$);
								}
			;
ExtDefList	:	ExtDef ExtDefList	{$$ = reduce("ExtDefList",2,$1,$2);}
			|	/*empty*/		{$$ = reduce("ExtDefList",0);}
			;
ExtDef		:	Specifier ExtDecList SEMI	{$$ = reduce("ExtDef",3,$1,$2,$3);}
			|	Specifier SEMI		{$$ = reduce("ExtDef",2,$1,$2);}
			|	Specifier FunDec CompSt	{$$ = reduce("ExtDef",3,$1,$2,$3);}
			|	Specifier FunDec SEMI	{$$ = reduce("ExtDef",3,$1,$2,$3);}
			;
ExtDecList	:	VarDec		{$$ = reduce("ExtDecList",1,$1);}
			|	VarDec COMMA ExtDecList	{$$ = reduce("ExtDecList",3,$1,$2,$3);}
			;

/*Specifier*/
Specifier	:	TYPE		{$$ = reduce("Specifier",1,$1);}
			|	StructSpecifier		{$$ = reduce("Specifier",1,$1);}
			;
StructSpecifier	:	STRUCT OptTag LC DefList RC		{$$ = reduce("StructSpecifier",5,$1,$2,$3,$4,$5);}
				|	STRUCT Tag		{$$ = reduce("StructSpecifier",2,$1,$2);}
				;
OptTag		:	ID		{$$ = reduce("OptTag",1,$1);}
			|	/*empty*/	{$$ = reduce("OptTag",0);}
			;
Tag			:	ID		{$$ = reduce("Tag",1,$1);}
			;

/*Declarators*/
VarDec		:	ID			{$$ = reduce("VarDec",1,$1);}
			|	VarDec LB INT RB	{$$ = reduce("VarDec",4,$1,$2,$3,$4);}
			;
FunDec		:	ID LP VarList RP	{$$ = reduce("FunDec",4,$1,$2,$3,$4);}
			|	ID LP RP			{$$ = reduce("FunDec",3,$1,$2,$3);}
			;
VarList		:	ParamDec COMMA VarList	{$$ = reduce("VarList",3,$1,$2,$3);}
			|	ParamDec		{$$ = reduce("VarList",1,$1);}
			;
ParamDec	:	Specifier VarDec	{$$ = reduce("ParamDec",2,$1,$2);}
			;

/*Statements*/
CompSt		:	LC DefList StmtList RC	{$$ = reduce("CompSt",4,$1,$2,$3,$4);}
			|	error RC
			;
StmtList	:	Stmt StmtList	{$$ = reduce("StmtList",2,$1,$2);}
			|	/*empty*/		{$$ = reduce("StmtList",0);}
			;
Stmt		:	Exp SEMI	{$$ = reduce("Stmt",2,$1,$2);}
			|	CompSt		{$$ = reduce("Stmt",1,$1);}
			|	RETURN Exp SEMI		{$$ = reduce("Stmt",3,$1,$2,$3);}
			|	IF LP Exp RP Stmt	%prec LOWER_THAN_ELSE	{$$ = reduce("Stmt",5,$1,$2,$3,$4,$5);}
			|	IF LP Exp RP Stmt ELSE Stmt		{$$ = reduce("Stmt",7,$1,$2,$3,$4,$5,$6,$7);}
			|	WHILE LP Exp RP Stmt	{$$ = reduce("Stmt",5,$1,$2,$3,$4,$5);}
			|	error SEMI
			;

/*Local Definitions*/
DefList		:	Def DefList		{$$ = reduce("DefList",2,$1,$2);}
			|	/*empty*/		{$$ = reduce("DefList",0);}
			;
Def			:	Specifier DecList SEMI	{$$ = reduce("Def",3,$1,$2,$3);}
			;
DecList		:	Dec					{$$ = reduce("DecList",1,$1);}
			|	Dec COMMA DecList	{$$ = reduce("DecList",3,$1,$2,$3);}
			;
Dec			:	VarDec				{$$ = reduce("Dec",1,$1);}
			|	VarDec ASSIGNOP Exp	{$$ = reduce("Dec",3,$1,$2,$3);}
			;

/*Expressions*/
Exp			:	Exp ASSIGNOP Exp	{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp AND Exp		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp OR Exp		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp RELOP Exp	{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp PLUS Exp	{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp MINUS Exp	{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp STAR Exp	{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp DIV Exp		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	LP Exp RP		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	MINUS Exp		{$$ = reduce("Exp",2,$1,$2);}
			|	NOT Exp			{$$ = reduce("Exp",2,$1,$2);}
			|	ID LP Args RP	{$$ = reduce("Exp",4,$1,$2,$3,$4);}
			|	ID LP RP		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	Exp LB Exp RB	{$$ = reduce("Exp",4,$1,$2,$3,$4);}
			|	Exp DOT ID		{$$ = reduce("Exp",3,$1,$2,$3);}
			|	ID				{$$ = reduce("Exp",1,$1);}
			|	INT				{$$ = reduce("Exp",1,$1);}
			|	FLOAT			{$$ = reduce("Exp",1,$1);}
			|	error RP
			;
Args		:	Exp COMMA Args	{$$ = reduce("Args",3,$1,$2,$3);}
			|	Exp				{$$ = reduce("Args",1,$1);}
			;

%%

treenode* reduce(char *name,int num,...){
	//printf("%s takes reduce\n",name);
	va_list ap;
	int i = 0;
	treenode *p;
	p = malloc(sizeof(treenode));
	if(p == NULL){
		yyerror("no space");
		exit(5);
	}
	p->node = malloc(num * sizeof(treenode));
	if(p->node == NULL){
		yyerror("no sapce");
		exit(6);
	}
	p->type = NODE_NOTERMINUS;
	p->lineno = 0;
	strcpy(p->name,name);
	p->child_num = num;
	va_start(ap,num);
	for(; i < num;i++){
		//printf("construct child node\n");
		p->node[i] = va_arg(ap,treenode*);
		//printf("construct child node done!\n");
	}
	if(num != 0)
		p->lineno = p->node[0]->lineno;
	va_end(ap);
	return p;
}


void returnspace(treenode *root){
	int i = 0;
	if(root->type = NODE_NOTERMINUS){
		for(;i < root->child_num;i++)
			returnspace(root->node[i]);
		free(root->node);
	}else
		free(root);
	
}


yyerror(char* msg){
	g_error = 1;
	fprintf(stderr,"Error type B at line %d :%s\n",yylineno,msg);
}

