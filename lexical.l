%{
	#include "stdio.h"
	#include "stdlib.h"
	#include "string.h"
	#include "syntax.tab.h"
	#include "header/treenode.h"
	
	int yycolumn = 1;

	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
	yylloc.first_column = yycolumn;yylloc.last_column = yycolumn + yyleng - 1;\
	yycolumn += yyleng;

	treenode* intnode(int value,int line);
	treenode* floatnode(float value,int line);
	treenode* idnode(char *id,int line);
	treenode* opernode(char *terminus,int line,char *text);
%}
%option yylineno

digit		[0-9]
letter		[_a-zA-Z]
id			[_a-zA-Z]([0-9]|[_a-zA-Z])*
int			([1-9][0-9]*)|0
float		[1-9][0-9]*\.[0-9]*|0\.[0-9]+
relop		>|<|>=|<=|==|!=
whitespace	[ \t]
newline		[\r\n]
%%

{whitespace}	{	}
{newline}	{	yycolumn = 1;}

{int}		{	
				yylval.type_node = intnode(atoi(yytext),yylineno);
				return INT;
			}

{float}		{	
				yylval.type_node = floatnode(atof(yytext),yylineno);
				return FLOAT;
			}

";"			{	
				yylval.type_node = opernode("SEMI",yylineno,yytext);
				return SEMI;		
			}
","			{	
				yylval.type_node = opernode("COMMA",yylineno,yytext);
				return COMMA;	
			}
"="			{	
				yylval.type_node = opernode("ASSIGNOP",yylineno,yytext);
				return ASSIGNOP;
			}
{relop}		{	
				yylval.type_node = opernode("RELOP",yylineno,yytext);
				return RELOP;	
			}
"+"			{	
				yylval.type_node = opernode("PLUS",yylineno,yytext);
				return PLUS;
			}
"-"			{	
				yylval.type_node = opernode("MINUS",yylineno,yytext);
				return MINUS;		
			}
"*"			{	
				yylval.type_node = opernode("STAR",yylineno,yytext);
				return STAR;
			}
"/"			{	
				yylval.type_node = opernode("DIV",yylineno,yytext);
				return DIV;		
			}
"&&"		{	
				yylval.type_node = opernode("AND",yylineno,yytext);
				return AND;	
			}
"||"		{	
				yylval.type_node = opernode("OR",yylineno,yytext);
				return OR;		
			}
"//"		{
				char c = input();
				while(c != '\n' && c != EOF)
					c = input();
			}
"/*"		{
				/*char c;
				while((c = input()) != '*' && c != EOF){}
				if(c == '*')
					while((c = input()) == '*'){}
				if(c == '/')
					break;
				if(c == EOF){
					exit(0);
				}*/
				char c;
				while((c = input()) != EOF){
					if( c != EOF && c == '*')
					{
						char temp = input();
						if(temp != EOF && temp == '/')
							break;
					}
				}
			}
"."			{	
				yylval.type_node = opernode("DOT",yylineno,yytext);
				return DOT;	
			}
"!"			{	
				yylval.type_node = opernode("NOT",yylineno,yytext);
				return NOT;			
			}
"("			{	
				yylval.type_node = opernode("LP",yylineno,yytext);
				return LP;			
			}
")"			{	
				yylval.type_node = opernode("RP",yylineno,yytext);
				return RP;			
			}
"["			{	
				yylval.type_node = opernode("LB",yylineno,yytext);
				return LB;			
			}
"]"			{	
				yylval.type_node = opernode("RB",yylineno,yytext);
				return RB;			
			}
"{"			{	
				yylval.type_node = opernode("LC",yylineno,yytext);
				return LC;			
			}
"}"			{	
				yylval.type_node = opernode("RC",yylineno,yytext);
				return RC;			
			}
"struct"	{	
				/*printf("parse struct\n");*/
				yylval.type_node = opernode("STRUCT",yylineno,yytext);
				return STRUCT;		
			}
"return"	{	
				yylval.type_node = opernode("RETURN",yylineno,yytext);
				return RETURN;		
			}
"if"		{	
				yylval.type_node = opernode("IF",yylineno,yytext);
				return IF;		
			}
"else"		{	
				yylval.type_node = opernode("ELSE",yylineno,yytext);
				return ELSE;		
			}
"while"		{	
				yylval.type_node = opernode("WHILE",yylineno,yytext);
				return WHILE;		
			}
"int"		{	
				/*printf("parse int\n");*/
				yylval.type_node = opernode("INT",yylineno,yytext);
				return TYPE;		
			}
"float"		{	
				yylval.type_node = opernode("FLOAT",yylineno,yytext);
				return TYPE;	
			}

{id}		{	
				yylval.type_node = idnode(yytext,yylineno);
				return ID;		
			}

.			{	
				g_error = 1;
				printf("Error type A at line %d: Mysterious character \'%s\'\n",yylineno,yytext);
			}


%%

treenode* intnode(int value,int line){
	treenode *p;
	p = malloc(sizeof(treenode));
	if( p == NULL ){
		yyerror("no space");
		exit(1);
	}else{
		p->type = NODE_INT;
		p->lineno = line;
		p->ival = value;
	}
	return p;
}

treenode* floatnode(float value,int line){
	treenode *p;
	p = malloc(sizeof(treenode));
	if(p == NULL){
		yyerror("no space");
		exit(2);
	}else{
		p->type = NODE_FLOAT;
		p->lineno = line;
		p->fval = value;
	}
	return p;
}

treenode* idnode(char *id,int line){
	treenode *p;
	p = malloc(sizeof(treenode));
	if( p == NULL){
		yyerror("no space");
		exit(3);
	}else{
		p->type = NODE_ID;
		p->lineno = line;
		strcpy(p->idval,id);
		strcpy(p->name,id);
	}
	return p;
}

treenode* opernode(char *terminus,int line,char *text){
	treenode *p;
	p = malloc(sizeof(treenode));
	if( p == NULL){
		yyerror("no space");
		exit(4);
	}else{
		p->type = NODE_TERMINUS;
		p->lineno = line;
		strcpy(p->operval,terminus);
		strcpy(p->name,text);
	}
	return p;
}
