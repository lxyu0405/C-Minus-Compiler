#ifndef _HASH_C_
#define _HASH_C_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/symbol.h"
#include "../header/treenode.h"
#include "../header/debug.h"

#define HASH_SIZE 2048				

//hash.c
void initHashList();
uint hash_pjw(char *name);						//P.J.Weinberger hash function

int equalArrayType(Type array1,Type array2);
int equalType(Type type1,Type type2);
int equalArgsType(FieldList func,FieldList args);
int equalExternArgs(FieldList func,FieldList extern_args);

FieldList fetchStruct(char *name);
FieldList fetchFunc(char *name);				//从函数符号表取出
FieldList fetchArg(char *name);					//从变量符号表取出

int argLegalName(char *name);
int insertStruct(FieldList str);
int insertFunc(FieldList func);					//函数符号表插入
int insertArg(FieldList arg);					//变量符号表插入

void printTree(treenode* root,int layer);
void printHashList(FieldList *hashList,char *name);
void printArgs(FieldList arg,char *name);

void checkExtern();
void externFunc(FieldList func,int line);

/*===============================
*	typedef FieldList_* FieldList ;
*	FieldList:
*			{
*				char *name;
*				Type type;
*				FieldList tail;
*			}
=================================*/


FieldList argList[HASH_SIZE];					//变量符号表
FieldList funcList[HASH_SIZE];					//函数符号表
FieldList structList[HASH_SIZE];				//struct类型符号表
int funcFlag[HASH_SIZE];						//-1——空 0——定义 其他——行号
//ExternList externList[HASH_SIZE];

void initHashList()
{
	int i = 0;
	for(; i < HASH_SIZE;i++)
	{
		argList[i] = NULL;
		funcList[i] = NULL;
		structList[i] = NULL;
		funcFlag[i] = -1;
	}
}


void printHashList(FieldList *hashList,char *name)
{
	//deprintf("enter function printHashList()\n");
	printf("%s:\n",name);
	int i = 0;
	int count;
	Type p;
	FieldList q;
	for(;i < HASH_SIZE;i++)
	{
		if(hashList[i] != NULL)
		{
			//deprintf("hashlist none empty found\n");
			FieldList temp = hashList[i];
			printf("\tID: %s\n",temp->name);
			while(temp != NULL)
			{
				switch(temp->type->kind)
				{
					case basic:
						if((temp->type->u).basic == 0)
							printf("\tType(basic): int\n");
						else if((temp->type->u).basic == 1)
							printf("\tType(basic): float\n");
						break;
					case array:
						printf("\tType(array) name: %s\n",temp->name);
						p = temp->type;
						while(p->kind == array)
						{
							printf("\tType(array): size %d\n",(p->u).array.size);
							p = (p->u).array.elem;
						}
						if(p->kind == basic){
							if((p->u).basic == 0)
								printf("\tType(array_int)\n");
							else if((p->u).basic == 1)
								printf("\tType(array_float)\n");
						}
						if(p->kind == structure){
							printf("\tType(struct):%s\n",p->u.structure->name);
						}
						break;
					case structure:
						if(strcmp(name,"structList") != 0)
							printf("\tType(struct) name: %s\n",(temp->type->u).structure->name);
						count = 0;
						q = temp->tail;
						while(q != NULL)
						{
							count++;
							printf("\tType(struct): field %d name %s kind %d\n",count,q->name,q->type->kind);
							q = q->tail;
						}
				}
				temp = temp->tail;
			}
		}
	}
}

int argLegalName(char *name)
{
	int i = 0;
	for(;i < HASH_SIZE;i++)
	{
		if(structList[i] != NULL)
			if(strcmp(structList[i]->name,name) == 0)
				return 0;
		if(argList[i] != NULL)
			if(strcmp(argList[i]->name,name) == 0)
				return 0;
	}
	return 1;

}

void printTree(treenode* root,int layer)
{
	int count = 0;
	int i = 0;
	//printf("type:\n");
	//printf("root->type %d\n",root->type);
	//if(root->type == NODE_NOTERMINUS)
	//	printf("noterminus\n");
	//printf("get type!\n");
	if( root->type == NODE_NOTERMINUS && root->child_num == 0)
		return ;
	for(;count < layer;count++)
		printf("  ");
	//printf("printf whitespace done!\n");
	switch(root->type){
		case NODE_INT:
			//printf("start to print int\n");
			printf("INT: %d\n",root->ival);
			break;
		case NODE_FLOAT:
			//printf("start to print float\n");
			printf("FLOAT: %lf\n",root->fval);
			break;
		case NODE_ID:
			//printf("start to print id\n");
			printf("ID: %s\n",root->idval);
			break;
		case NODE_TERMINUS:
			//printf("start to print terminus\n");
			if(strcmp(root->operval,"INT") == 0)
				printf("TYPE: int\n");
			else if(strcmp(root->operval,"FLOAT") == 0)
				printf("TYPE: float\n");
			else
				printf("%s\n",root->operval);
			break;
		case NODE_NOTERMINUS:
			//printf("start to print noterminus\n");
			printf("%s (%d)\n",root->name,root->lineno);
			layer++;
			for(;i < root->child_num;i++){
				//printf("print layer %d\n",layer++);
				printTree(root->node[i],layer);
				//printf("print layer %d done!\n",layer++);
			}
	}
}

uint hash_pjw(char *name)			//P.J.Weinberger hash function
{
	uint val = 0, i;
	for(; *name; ++name)
	{
		val = (val << 2) + *name;
		if(i = val & ~0x3fff)
			val = (val ^ (i >> 12))& 0x3fff;
	}
	return val;
}

int insertArg(FieldList arg)		//变量符号表插入
{
	//deprintf("insertArg() '%s'\n",arg->name);
	if(argLegalName(arg->name) == 0)
		return 0;
	uint index = hash_pjw(arg->name) % HASH_SIZE;
	FieldList p, q;
	p = argList[index];
	if( p == NULL )
	{
		argList[index] = arg;
		arg->tail = NULL;
		return 1;
	}
	else
	{
		for(;p != NULL;p = p->tail)
		{
			q = p;
			if(strcmp(p->name,arg->name) == 0)
				return 0;
		}
		arg->tail = NULL;
		q->tail = arg;
		return 1;
	}
}

int insertFunc(FieldList func)		//函数符号表插入
{
	//deprintf("insertFunc() '%s'\n",func->name);
	uint index = hash_pjw(func->name) % HASH_SIZE;
	int i = 0;
	FieldList p;
	if(funcList[index] == NULL)
	{
		//deprintf("find empty FieldList\n");
		funcList[index] = func;
		funcFlag[index] = 1;
		return 1;
	}
	if(strcmp(funcList[index]->name,func->name) == 0)
	{
		if(funcFlag[index] == 1)
			return 0;
		else if(funcFlag[index] != -1)
		{
			if(equalExternArgs(func,funcList[index]) == 0)
			{
				funcList[index] = func;
				printf("Error type 19 at line %d: Inconsistent declaration of function '%s'\n",funcFlag[index],func->name);
				funcFlag[index] = 1;
				return 2;
			}
			else
			{
				funcFlag[index] = 1;
				return 1;
			}
		}
	}
	for(i = index + 1;i < HASH_SIZE;i = (i+1) % HASH_SIZE)
	{
		if(funcList[i] == NULL)
		{
			funcList[i] = func;
			funcFlag[i] = 1;
			return 1;
		}
		else if(strcmp(funcList[i]->name,func->name) == 0)
		{
			if(funcFlag[i] == 1)
				return 0;
			else if(funcFlag[i] != 1)
			{
				if(equalExternArgs(func,funcList[i]) == 0)
				{
					funcList[i] = func;
					printf("Error type 19 at line %d: Inconsistent declaration of function '%s'\n",funcFlag[i],func->name);
					funcFlag[i] = 1;
					return 2;
				}
				else
				{
					funcFlag[i] = 1;
					return 1;
				}
			}
		}
	}
}

void externFunc(FieldList func,int line)		//插入函数声明
{
	//deprintf("externFunc() '%s'\n",func->name);
	uint index = hash_pjw(func->name) % HASH_SIZE;
	int i = 0;
	FieldList p;
	if(funcList[index] == NULL)
	{
		//deprintf("find empty FieldList line:%d\n",line);
		funcList[index] = func;
		funcFlag[index] = line+1;
		return ;
	}
	if(strcmp(funcList[index]->name,func->name) == 0)
	{
		if(equalExternArgs(funcList[index],func) == 0)
		{
			printf("Error type 19 at line %d: Inconsistent declaration of function '%s'\n",line,func->name);
			return ;
		}
		else
		{
			if(funcFlag[index] == 1)
				return ;
			else 
				return ;
		}
	}
	for(i = index + 1;i < HASH_SIZE;i = (i+1) % HASH_SIZE)
	{
		if(funcList[i] == NULL)
		{
			funcList[i] = func;
			funcFlag[i] = line+1;
			return ;
		}
		else if(strcmp(funcList[i]->name,func->name) == 0)
		{
			if(equalExternArgs(funcList[i],func) == 0)
			{
				printf("Error type 19 at line %d: Inconsistent declaration of function '%s'\n",line,func->name);
				return ;
			}
			else
				return;
		}
	}
}

void checkExtern()
{
	int i = 0;
	for(;i < HASH_SIZE;i++)
	{
		if(funcFlag[i] != -1)
			if(funcFlag[i] != 1)
				printf("Error type 18 at line %d: Undefined function '%s'\n",funcFlag[i]-1,funcList[i]->name);
	}
}

int insertStruct(FieldList str)
{
	if(strcmp(str->name,"0null0") == 0)
	{
		int i = 0;
		uint index = hash_pjw(str->name) % HASH_SIZE;
		for(i = index;i < HASH_SIZE;i = (i+1)%HASH_SIZE)
			if(structList[i] == NULL)
			{
				structList[i] = str;
				return 1;
			}
	}
	else
	{
		//deprintf("insertStruct() '%s'\n",str->name);
		if(argLegalName(str->name) == 0)
			return 0;
		uint index = hash_pjw(str->name) % HASH_SIZE;
		uint i = 0;
		FieldList p;
		if(structList[index] == NULL)
		{
			structList[index] = str;
			return 1;
		}
		if(strcmp(structList[index]->name,str->name) == 0)
			return 0;
		for(i = index + 1;i < HASH_SIZE;i = (i + 1) % HASH_SIZE)
		{
			if(structList[i] == NULL)
			{
				structList[i] = str;
				return 1;
			}
			else if(strcmp(structList[i]->name,str->name) == 0)
				return 0;
		}
	}
}
FieldList fetchArg(char *name)		//从变量符号表取出
{
	uint index = hash_pjw(name) % HASH_SIZE;
	FieldList p = argList[index];
	for(; p != NULL;p = p->tail)
		if(strcmp(p->name,name) == 0)
			return p;
	return NULL;
}

FieldList fetchFunc(char *name)		//从函数符号表取出
{
	uint index = hash_pjw(name) % HASH_SIZE;
	uint i = index;
	do{
		if(funcList[i] != NULL)
			if(strcmp(funcList[i]->name,name) == 0)
				return funcList[i];
		i = (i + 1) % HASH_SIZE;
	}while(i != index);
	return NULL;
}

FieldList fetchStruct(char *name)
{
	uint index = hash_pjw(name) % HASH_SIZE;
	uint i = index;
	do{
		if(structList[i] != NULL)
			if(strcmp(structList[i]->name,name) == 0)
				return structList[i];
		i = (i + 1) % HASH_SIZE;
	}while(i != index);
	return NULL;
}

void printArgs(FieldList arg,char *name)
{
	//deprintf("printArgs()\n");
	int count = 0;
	FieldList p = arg;
	while( p!= NULL)
	{
		count++;
		printf("%d %s's type kind: %d \n",count,name,p->type->kind);
		p = p->tail;
	}
}

int equalExternArgs(FieldList func,FieldList extern_args)
{
	FieldList args1, args2;
	args1 = func;
	args2 = extern_args;
	//printArgs(args1,"args1");
	//printArgs(args2,"args2");
	for(;args1 != NULL && args2 != NULL;args1 = args1->tail,args2 = args2->tail)
	{
		if(args1->type->kind != args2->type->kind || strcmp(args1->name,args2->name) != 0)
			return 0;
		else	// args1->type->kind == args2->type->kind
		{
			switch(args1->type->kind)
			{
				case basic:	
					if((args1->type->u).basic != (args2->type->u).basic)
						return 0;
					break;
				case array:
					if(equalArrayType(args1->type,args2->type) == 0)
						return 0;
					break;
				case structure:
					if(strcmp(args1->name,args2->name) != 0)
						return 0;
					break;
			}
		}
	}
	if(!(args1 == NULL && args2 == NULL))
		return 0;
	return 1;	
}

int equalArgsType(FieldList func,FieldList args)
{
	FieldList args1, args2;
	args1 = func;
	args2 = args;
	//printArgs(args1,"args1");
	//printArgs(args2,"args2");
	for(;args1 != NULL && args2 != NULL;args1 = args1->tail,args2 = args2->tail)
	{
		if(args1->type->kind != args2->type->kind)
			return 0;
		else	// args1->type->kind == args2->type->kind
		{
			switch(args1->type->kind)
			{
				case basic:	
					if((args1->type->u).basic != (args2->type->u).basic)
						return 0;
					break;
				case array:
					if(equalArrayType(args1->type,args2->type) == 0)
						return 0;
					break;
				case structure:
					if(strcmp((args1->type->u).structure->name,(args2->type->u).structure->name) != 0)
						return 0;
					break;
			}
		}
	}
	if(args1 == NULL && args2 == NULL)
		return 1;
	else
		return 0;
}


int equalType(Type type1,Type type2)
{
	if(type1 == NULL || type2 == NULL)
		return 0;
	else if(type1->kind != type2->kind)
		return 0;
	else 
	{
		switch(type1->kind)
		{
			case basic:
				if((type1->u).basic != (type2->u).basic)
					return 0;
				else
					return 1;
				break;
			case array:
				if(equalArrayType(type1,type2) == 0)
					return 0;
				else
					return 1;
				break;
			case structure:
				if(strcmp((type1->u).structure->name,(type2->u).structure->name) != 0)
					return 0;
				else
					return 1;
		}
	}
}

int equalArrayType(Type array1,Type array2)
{
	int dimen1 = 0;
	int dimen2 = 0;
	Type p1 = array1;
	Type p2 = array2;
	while(p1->kind == array)
	{
		dimen1++;
		p1 = (p1->u).array.elem;
	}
	while(p2->kind == array)
	{
		dimen2++;
		p2 = (p2->u).array.elem;
	}
	if(dimen1 != dimen2)
		return 0;
	else if(dimen1 == dimen2 && (p1->u).basic != (p2->u).basic)
		return 0;
	else
		return 1;
}
#endif
