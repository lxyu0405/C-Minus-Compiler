#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/treenode.h"
#include "../header/symbol.h"
#include "../header/debug.h"
#include "hash.c"


//semantic.c
extern void analyze(treenode *root);
void analyzeDef(treenode *def);
Type getType(treenode *spec);
void analyzeStructDefList(treenode *struct_node,FieldList struct_field);
void analyzeStructDecList(treenode *DecList,FieldList struct_field,Type struct_spec_type);
void analyzeVarDec(treenode *VarDec,FieldList new_var);
void analyzeStmt(treenode *Stmt);
void analyzeExtDecList(treenode *DecList,Type spec_type);
void analyzeDecList(treenode *DecList,Type spec_type);
void analyzeFuncPara(treenode *VarList,FieldList para_field);
void analyzeArgs(treenode *args);

int funcPara(char *name);

Type getExpType(treenode *exp);
Type getIDType(treenode *id);
Type getMathType(treenode *exp);
void analyzeLeftAssign(treenode *exp);

/*============Treenode Struct==================
+	treenode
+	{
+		node_type type;
+		int lineno;
+		union
+		{
+			int ival;
+			float fval;
+			char idval[30];
+			char operval[30];
+			char name[30];
+		}
+		int child_num;
+		treenode **node;
+	}
================================================*/

Type g_funcType = NULL;					//存储函数类型
FieldList g_newFunc = NULL;					//记录函数参数
FieldList g_expArgs = NULL;					//存储实参
FieldList g_paraField = NULL;
FieldList g_structField = NULL;

void analyze(treenode *root)				//对语法树进行语义分析
{
	//deprintf("enter Function: analyze()\n");
	treenode *p;
	int i = 0;
	//deprintf("root->name = %s\n",root->name);
	for(;i < root->child_num;i++)
	{
		p = root->node[i];
		if(p != NULL)
		{
			//deprintf("not NULL node->name = %s\n",p->name);
			if(strcmp(p->name,"Def") == 0 || strcmp(p->name,"ExtDef") == 0)
			{
				g_structField = NULL;
				//deprintf("find Def\n");
				analyzeDef(p);			//analyze definitions
			}
			else if(strcmp(p->name,"Stmt") == 0)
				analyzeStmt(p);		//Others
			else
				analyze(p);
		}
	}
}

void analyzeDef(treenode *def)
{
	//deprintf("enter Function: analyzeDef()\n");
	treenode* Spec_node = def->node[0];					//Specifier
	treenode* DecList_node = def->node[1];				//DecList
	
	//	Specifier
	Type Spec_type = getType(Spec_node);

	//	ExtDef	->	Specifier ExtDecList SEMI
	if(strcmp(DecList_node->name,"ExtDecList") == 0)
	{
		//	ExtDecList	->	VarDec
		//				|	VarDec COMMA ExtDecList
		analyzeExtDecList(DecList_node,Spec_type);
	}
	//	Def	->	Specifier DecList SEMI
	else if(strcmp(DecList_node->name,"DecList") == 0)
	{
		//	DecList	->	Dec
		//			|	Dec COMMA DecList
		//	Dec	->	VarDec
		//		|	VarDec ASSIGNOP Exp
		analyzeDecList(DecList_node,Spec_type);
	}
	//	ExtDef	->	Specifier FunDec CompSt
	//			|	Specifier FunDec SEMI
	else if(strcmp(DecList_node->name,"FunDec") == 0)
	{
		//	FunDec	->	ID ( VarList )
		//			|	ID ( )
		//	VarList	->	ParamDec COMMA VarList
		//			|	ParamDec
		//	ParamDec	->	Specifier VarDec
		//deprintf("FunDec\n");
		treenode *ID_node = DecList_node->node[0];
		g_newFunc = (FieldList)malloc(sizeof(struct FieldList_));
		g_newFunc->name = ID_node->idval;
		g_newFunc->type = Spec_type;
		//存储函数类型
		g_funcType = Spec_type;

		if(DecList_node->child_num == 4)
		{	//	->	ID ( VarList )
			treenode *VarList = DecList_node->node[2];
			//FieldList Para_field = NULL;
			analyzeFuncPara(VarList,g_paraField);
			g_newFunc->tail = g_paraField;
			g_paraField = NULL;
			//printf("+++++++++name '%s'\n",g_paraField->name);
		}
		else
		{	//	->	ID ()
			//deprintf("test Function main\n");
			g_newFunc->tail = NULL;
		}
		//printHashList(funcList);
		treenode *tail = def->node[2];
		//printf("$$$$$$$$$$$$$$$$$$$$name: %s\n",tail->name);
		if(strcmp(tail->name,"CompSt") == 0)
		{
			//deprintf("new function: %s\n",g_newFunc->name);
			if(insertFunc(g_newFunc) == 0)
			{
				printf("Error type 4 at line %d: Redefined function '%s' \n",DecList_node->lineno,g_newFunc->name);
			}
			analyze(def->node[2]);
		}
		else
		{
			//printf("extern at line %d\n",DecList_node->lineno);
			externFunc(g_newFunc,DecList_node->lineno);
		}
	}
}

void analyzeFuncPara(treenode *VarList,FieldList para_field)
{	//	->	para ,	varlist
	//	|	para
	treenode *ParamDec_node = VarList->node[0];
	// para	->	specifier vardec
	treenode *Spec_node = ParamDec_node->node[0];
	treenode *VarDec_node = ParamDec_node->node[1];
	
	Type para_type = getType(Spec_node);
	FieldList new_Para = (FieldList)malloc(sizeof(struct FieldList_));
	analyzeVarDec(VarDec_node,new_Para);
	//printf("$$$$$$$new_para id '%s'\n",new_Para->name);
	if(	new_Para->type == NULL)
	{
		new_Para->type = para_type;
	}
	else
	{
		Type p = new_Para->type;
		while((p->u).array.elem != NULL)
			p = (p->u).array.elem;
		(p->u).array.elem = para_type;
	}
	
	if(g_paraField == NULL)
	{
		new_Para->tail = NULL;
		g_paraField = new_Para;
		//printf("$$$$$$$new_para id '%s'\n",g_paraField->name);
	}
	else
	{
		FieldList p = g_paraField;
		for(;p->tail != NULL;p = p->tail);
		new_Para->tail = NULL;
		p->tail = new_Para;
	}
	if(VarList->child_num == 3)
		analyzeFuncPara(VarList->node[2],para_field);
}


void analyzeDecList(treenode *DecList,Type spec_type)
{
	//printHashList(argList,"argList");
	//deprintf("enter Function analyzeDecList()\n");
	treenode *Dec_node = DecList->node[0];
	treenode *VarDec_node = Dec_node->node[0];
	FieldList new_VarDec = (FieldList)malloc(sizeof(struct FieldList_));

	analyzeVarDec(VarDec_node,new_VarDec);
	if(Dec_node->child_num == 3)
	{
		Type temp = getExpType(Dec_node->node[2]);
		if(equalType(spec_type,temp) == 0)
			;
			//printf("Error type 5 at line %d: Type mismatched\n",Dec_node->lineno);
	}
	if(new_VarDec->type == NULL)
	{
		new_VarDec->type = spec_type;
	}
	else
	{
		Type p = new_VarDec->type;
		while((p->u).array.elem != NULL)
			p = (p->u).array.elem;
		(p->u).array.elem = spec_type;
	}
	if(insertArg(new_VarDec) == 0)
	{
		printf("Error type 3 at line %d: Redefined variable name '%s'\n",VarDec_node->lineno,new_VarDec->name);
	}
	if(DecList->child_num == 3)
		analyzeDecList(DecList->node[2],spec_type);
}

void analyzeExtDecList(treenode *DecList,Type spec_type)
{
	//	ExtDecList	->	VarDec
	//				|	VarDec COMMA ExtDecList
	treenode *VarDec_node = DecList->node[0];
	FieldList new_VarDec = (FieldList)malloc(sizeof(struct FieldList_));
	analyzeVarDec(VarDec_node,new_VarDec);
	
	if(new_VarDec->type == NULL)
	{
		new_VarDec->type = spec_type;
	}
	else
	{
		Type p = new_VarDec->type;
		while((p->u).array.elem != NULL)
			p = (p->u).array.elem;
		(p->u).array.elem = spec_type;
	}
	if(insertArg(new_VarDec) == 0)
	{
		printf("Error type 3 at line %d: Redefined variable name '%s'\n",VarDec_node->lineno,new_VarDec->name);
	}	
	if(DecList->child_num == 3)
		analyzeExtDecList(DecList->node[2],spec_type);
}

Type getType(treenode *spec)
{
	//deprintf("enter Function getType()\n");
	treenode* type_node = spec->node[0];		//TYPE	|	StructSpecifier
	
	Type basic_type, struct_type;
	basic_type = (Type)malloc(sizeof(struct Type_));
	basic_type->kind = basic;
	struct_type = (Type)malloc(sizeof(struct Type_));
	struct_type->kind = structure;
	//deprintf("save space for type\n");
	if(strcmp(type_node->operval,"INT") == 0)
	{	//basic 0-int
		//deprintf("find int type\n");
		(basic_type->u).basic = 0;
		return basic_type;
	}
	else if(strcmp(type_node->operval,"FLOAT") == 0)
	{	//basic 1-float
		//deprintf("find float type\n");
		(basic_type->u).basic = 1;
		return basic_type;
	}
	else if(strcmp(type_node->name,"StructSpecifier") == 0)
	{	//struct type
		treenode *Tag_node = type_node->node[1];
		if(strcmp(Tag_node->name,"OptTag") == 0)
		{	//	StructSpecifier	->	STRUCT OptTag { DefList }
			treenode *DefList_node = type_node->node[3];
			//g_structField = (FieldList)malloc(sizeof(struct FieldList_));
			FieldList current_struct = (FieldList)malloc(sizeof(struct FieldList_));
			//Type temp_type = (Type)malloc(sizeof(struct Type_));
			Type current_type = (Type)malloc(sizeof(struct Type_));
			//g_structField->tail = NULL;
			current_struct->tail = NULL;

			analyzeStructDefList(DefList_node,current_struct);
			
			//g_structField->type = temp_type;
			//g_structField->type->kind = structure;
			current_struct->type = current_type;
			current_struct->type->kind = structure;
			
			if(Tag_node->child_num == 1)
			{	//OptTag/Tag	->	ID
				treenode *ID_node = Tag_node->node[0];
				//g_structField->name = ID_node->idval;
				current_struct->name = ID_node->idval;
				//printArgs(g_structField,"struct");
				if(insertStruct(current_struct) == 0)
				{
					printf("Error type 16 at line %d: Duplicated name '%s'\n",ID_node->lineno,ID_node->idval);
					return NULL;
				}
				else
				{
					//(struct_type->u).structure = g_structField;
					(struct_type->u).structure = current_struct;
					return struct_type;
				}
			}
			else if(Tag_node->child_num == 0)
			{	//empty
				char *name = malloc(sizeof(10));
				strcpy(name,"0null0");
				//g_structField->name = name;
				current_struct->name = name;
				//insertStruct(g_structField);
				insertStruct(current_struct);
				//(struct_type->u).structure = g_structField;
				(struct_type->u).structure = current_struct;
				return struct_type;
			}
		}
		else	//	StructSpecifier	->	STRUCT Tag
		{	//	Tag	->	ID
			treenode *ID_node = Tag_node->node[0];
			FieldList checkID = fetchStruct(ID_node->idval);
			if(checkID == NULL)
				printf("Error type 17 at line %d: Undefined struct '%s'\n",ID_node->lineno,ID_node->idval);
			//	build struct_type
			//deprintf("STRUCT Tag\n");
			(struct_type->u).structure = checkID;
			return struct_type;
		}
	}
	else
	{
		//printf("Error: getType() not found type\n");
		return NULL;
	}
}



void analyzeStructDefList(treenode *struct_node,FieldList struct_field)
{
	if(struct_node->child_num == 2)	
	{	//	DefList	->	Def DefList
		treenode *Def_node = struct_node->node[0];
		treenode *DefList_node = struct_node->node[1];
		//	Def	->	Specifier DecList SEMI 
		treenode *Spec_node = Def_node->node[0];
		treenode *DecList_node = Def_node->node[1];
		
		Type struct_spec_type = getType(Spec_node);
		analyzeStructDecList(DecList_node,struct_field,struct_spec_type);
		//	new DefList
		analyzeStructDefList(DefList_node,struct_field);
	}
}

void analyzeStructDecList(treenode *DecList,FieldList struct_field,Type struct_spec_type)
{	
	//deprintf("enter function analyzeStructDecList()\n");
	//	DecList	->	Dec	|	Dec COMMA DecList
	treenode *Dec_node = DecList->node[0];
	//	Dec	->	VarDec	|	VarDec ASSIGNOP Exp
	treenode *VarDec_node = Dec_node->node[0];
	
	if(Dec_node->child_num == 3)
	{
		printf("Error type 15 at line %d: illegal initialization\n",Dec_node->lineno);
	}

	FieldList new_VarDec = (FieldList)malloc(sizeof(struct FieldList_));
	analyzeVarDec(VarDec_node,new_VarDec);
	//printf("new field name: %s \n",new_VarDec->name);	
	if(new_VarDec->type == NULL)
	{
		//deprintf("newVarDec is NULL\n");
		new_VarDec->type = struct_spec_type;
		//FieldList p = g_structField->tail;
		//deprintf("after_ewVarDec is NULL\n");
	}
	else
	{
		//deprintf("newVarDec is array\n");;
		Type p = new_VarDec->type;
		while((p->u).array.elem != NULL)
			p = (p->u).array.elem;
		(p->u).array.elem = struct_spec_type;
	}
	if(struct_field->tail == NULL)
	{
		new_VarDec->tail = NULL;
		//g_structField->tail = new_VarDec;
		struct_field->tail = new_VarDec;
	}
	else
	{
		//deprintf("g_structField another field\n");
		//FieldList p = g_structField->tail;
		FieldList p = struct_field->tail;
		FieldList pre_p = p;
		for(; p != NULL;p = p->tail)
		{
			pre_p = p;
			//printf("%d\n",strcmp(p->name,new_VarDec->name));
			//deprintf("###pre_before\n");
			if(strcmp(p->name,new_VarDec->name) == 0)
			{
				printf("Error type 15 at line %d: Redefined field '%s'\n",DecList->lineno,p->name);
				return;
			}
			//deprintf("###pre_after\n");
		}
		new_VarDec->tail = NULL;
		pre_p->tail = new_VarDec;
		//deprintf("insert another field\n");
	}
	//	DecList
	if(DecList->child_num == 3)
		analyzeStructDecList(DecList->node[2],struct_field,struct_spec_type);
}

void analyzeVarDec(treenode *VarDec,FieldList new_var)
{
	//deprintf("enter Function analyzeVarDec()\n");
	if(VarDec->child_num == 1)
	{	//	VarDec	->	ID
		treenode *ID_node = VarDec->node[0];
		new_var->name = ID_node->idval;
	}
	else
	{	//	VarDec	->	VarDec [ INT ]
		treenode *newVarDec = VarDec->node[0];
		analyzeVarDec(newVarDec,new_var);

		treenode *INT_node = VarDec->node[2];
		Type newType = (Type)malloc(sizeof(struct Type_));
		newType->kind = array;
		(newType->u).array.size = INT_node->ival;
		(newType->u).array.elem = NULL;

		if(new_var->type == NULL)
		{
			new_var->type = newType;
		}
		else
		{
			Type p = new_var->type;
			while((p->u).array.elem != NULL)
				p = (p->u).array.elem;
			(p->u).array.elem = newType;
		}
	}
}
//g_expArgs
void analyzeArgs(treenode *args)
{	//	->	Exp COMMA Args	|	Exp
	treenode *exp_node = args->node[0];
	//treenode *var_node = exp_node->node[0];
	int i = 0;
	FieldList temp = NULL;
	FieldList id = NULL;
	Type argType = getExpType(exp_node);
	if(argType != NULL)
	{
		temp = (FieldList)malloc(sizeof(struct FieldList_));
		temp->type = argType;
	}
	if(temp != NULL)
	{
		if(g_expArgs == NULL)
		{
			//deprintf("g_expArgs == NULL\n");
			g_expArgs = temp;
			//printArgs(g_expArgs,"g_expArgs_NULL");
		}
		else
		{
			//deprintf("g_expArgs != NULL\n");
			//printHashList(argList,"args_before");
			FieldList p = g_expArgs;
			for(; p->tail != NULL; p = p->tail);
			temp->tail = NULL;
			p->tail = temp;
			//printHashList(argList,"args_after");
		}
	}
	//printArgs(g_expArgs,"start_eload");
	if(args->child_num == 3)
	{
		//deprintf("another args\n");
		analyzeArgs(args->node[2]);
	}
	//printHashList(argList,"args_after");
}


void analyzeStmt(treenode *stmt)
{
	//deprintf("enter Function analyzeStmt()\n");
	Type temp = NULL;
	Type return_type = NULL;
	switch(stmt->child_num)
	{
		case 1://	CompSt
			//deprintf("CompSt\n");
			analyze(stmt->node[0]);
			break;
		case 2://	Exp SEMI
			//deprintf("Exp SEMI\n");
			temp = getExpType(stmt->node[0]);
			break;
		case 3://	RETURN Exp SEMI
			return_type = getExpType(stmt->node[1]);
			if(return_type == NULL || equalType(g_funcType,return_type) == 0)
				printf("Error type 8 at line %d: The return type mismatched\n",stmt->lineno);
			break;
		case 5://	IF ( Exp ) Stmt	|	WHILE ( Exp ) Stmt
			//deprintf("IF statement\n");
			temp = getExpType(stmt->node[2]);
			//deprintf("enter WHILE Stmt\n");
			analyze(stmt->node[4]);
			break;
		case 7://	IF ( Exp ) Stmt ELSE Stmt
			temp = getExpType(stmt->node[2]);
			analyze(stmt->node[4]);
			analyze(stmt->node[6]);
			break;
		//default://	Error	
			//printf("Error child number\n");
	}
}


Type getExpType(treenode *exp)
{
	//deprintf("enter Function getExpType()\n");
	if(exp->child_num == 1)//	-> ID | INT | FLOAT
	{
		//deprintf("child_num == 1\n");
		treenode *temp_node = exp->node[0];
		if(temp_node->type == NODE_INT)
		{	//INT
			Type int_type = (Type)malloc(sizeof(struct Type_));
			int_type->kind = basic;
			(int_type->u).basic = 0;
			return int_type;
		}
		else if(temp_node->type == NODE_FLOAT)
		{	//FLOAT
			Type float_type = (Type)malloc(sizeof(struct Type_));
			float_type->kind = basic;
			(float_type->u).basic = 1;
			return float_type;
		}
		else
		{	//ID
			//deprintf("name: %s is an id\n",exp->node[0]->idval);
			Type id_type = getIDType(temp_node);
			//if(id_type == NULL) deprintf("ID is NULL\n");
			return id_type;
		}
	}
	else if(exp->child_num == 2)//	-> MINUS Exp | NOT Exp
		return getExpType(exp->node[1]);
	else if(exp->child_num == 3)
	{
		//	->	Exp DOT ID
		if(strcmp(exp->node[1]->operval,"DOT") == 0)
		{
			//printf(".%s\n",exp->node[2]->idval);
			Type exp_type = getExpType(exp->node[0]);
			if(exp_type == NULL)
				return NULL;
			else if(exp_type->kind != structure)
			{
				printf("Error type 13 at line %d: Illegal use of '.'\n",exp->lineno);
				return NULL;
			}
			else
			{
				treenode *ID_node = exp->node[2];
				FieldList p = (exp_type->u).structure->tail;
				//p = (p->type->u).structure;
				//printf("%s's struct name: %s\n",exp->node[0]->node[0]->idval,(exp_type->u).structure->name);
				for(;p != NULL;p = p->tail)
				{
					//printf("\tcompare %s | %s\n",p->name,ID_node->idval);
					if(strcmp(p->name,ID_node->idval) == 0)
						return p->type;
				}
				printf("Error type 14 at line %d: Un-existed field '%s'\n",exp->lineno,ID_node->idval);
				return NULL;
			}
		}
		//	->	ID LP RP
		else if(strcmp(exp->node[1]->operval,"LP") == 0 && strcmp(exp->node[2]->operval,"RP") == 0)
		{
			treenode *ID_node = exp->node[0];
			int check1 = argLegalName(ID_node->idval);
			int check2 = funcPara(ID_node->idval);
			FieldList func = fetchFunc(ID_node->idval);
			if(check1 == 0 || check2 == 1)
			{
				printf("Error type 11 at line %d: '%s' must be a function\n",exp->lineno,ID_node->idval);
				return NULL;
			}
			else if(func != NULL)
			{
				if(func->tail != NULL)
					//printf("Error type 9 at line %d: The method '%s' is not applicable for the arguments\n",exp->lineno,func->name);
				return func->type;
			}
			else if(func == NULL)
			{
				printf("Error type 12 at line %d: Undefined function '%s'",exp->lineno,ID_node->idval);
				return NULL;
			}
		}
		//	-> LP Exp RP
		else if(strcmp(exp->node[0]->operval,"LP") == 0 && strcmp(exp->node[2]->operval,"RP") == 0)
			return getExpType(exp->node[1]);
		//	-> Exp operator Exp
		else if(exp->node[1]->type == NODE_TERMINUS)
		{
			Type math_type = getMathType(exp);
			return math_type;
		}
		
	}
	else if(exp->child_num == 4)
	{
		//	->	ID LP Args RP
		if(strcmp(exp->node[1]->operval,"LP") == 0 && strcmp(exp->node[3]->operval,"RP") == 0)
		{
			//printHashList(argList,"testArg");
			g_expArgs = NULL;
			treenode *ID_node = exp->node[0];
			int check1 = argLegalName(ID_node->idval);
			int check2 = funcPara(ID_node->idval);
			FieldList func = fetchFunc(ID_node->idval);
			//printArgs(func,"func");
			//deprintf("undefined function\n");
			if(check1 == 0 || check2 == 1)
			{
				printf("Error type 11 at line %d: '%s' must be a function\n",exp->lineno,ID_node->idval);
				return NULL;
			}
			else if(func == NULL)
			{
				printf("Error type 2 at line %d: Undefined function '%s'\n",exp->lineno,ID_node->idval);
				return NULL;
			}
			else if(func != NULL)	// var == NULL && func != NULL
			{
				analyzeArgs(exp->node[2]);
				if(equalArgsType(func->tail,g_expArgs) == 0)
				{
					//printf("Error type 9 at line %d: The method '%s' is not applicable for the arguments\n",exp->lineno,func->name);
					//deprintf("arg compare done!\n");
				}
				return func->type;
			}
		}

		//	->	Exp LB Exp RB
		else if(strcmp(exp->node[1]->operval,"LB") == 0 && strcmp(exp->node[3]->operval,"RB") == 0)
		{
			Type temp = NULL;
			treenode *exp_node = exp->node[0];
			if(exp_node->child_num == 1)
			{
				treenode *ID_node = exp_node->node[0];
				Type array_type = getExpType(exp_node);
				if(array_type == NULL || array_type->kind != array)
				{
					printf("Error type 10 at line %d: '%s' must be an array\n",exp->lineno,ID_node->idval);
					temp =  NULL;
				}
				else	
					temp = (array_type->u).array.elem;
			}
			else if(exp_node->child_num == 4)
				temp = getExpType(exp_node);
			else if(exp_node->child_num == 3){
				Type arr = getExpType(exp_node);
				while(arr->kind == array){
					arr = arr->u.array.elem;
				}
				return arr;
			}
			Type int_type = getExpType(exp->node[2]);
			if((int_type->u).basic != 0)
			{
				printf("Error type 12 at line %d: Operands type mistaken\n",exp->lineno);
			}
			return temp;
			
		}
	}
}

Type getMathType(treenode *exp)
{	//	exp	->	exp operator exp
	//deprintf("get math type\n");
	treenode *exp1 = exp->node[0];
	treenode *op = exp->node[1];
	treenode *exp2 = exp->node[2];
	Type type1 = getExpType(exp1);
	Type type2 = getExpType(exp2);
	if(strcmp(op->operval,"ASSIGNOP") == 0)
	{
		if(equalType(type1,type2) == 0)
		{
			//printf("Error type 5 at line %d: Type mismatched\n",exp->lineno);	
			return NULL;
		}
		else
		{
			if(exp1->child_num == 3 && strcmp(exp1->node[1]->operval,"ASSIGNOP") == 0)
			{
				return getExpType(exp1);
			}
			else
			{
				//deprintf("mark\n");
				analyzeLeftAssign(exp1);
				return type1;
			}
		}
	}
	else if(equalType(type1,type2) == 0 )
	{
		printf("Error type 7 at line %d: Operands type mismatched\n",exp->lineno);
		return NULL;
	
	}
	else
		return type1;
}

void analyzeLeftAssign(treenode *exp)
{
	if(exp->child_num == 1 && exp->node[0]->type == NODE_ID)
		;
	else
	{
		//printf("Error type 6 at line %d: The left-hand side of an assignment must be a variable\n",exp->lineno);
	}
}

int funcPara(char *name)
{
	if(g_newFunc == NULL)
		return 0;
	else
	{
		FieldList p = g_newFunc->tail;
		for(;p != NULL;p = p->tail)
			if(strcmp(p->name,name) == 0)
				return 1;
		return 0;
	}
}
Type getIDType(treenode *id)
{
	//deprintf("enter Function getIDType()\n");
	FieldList ID = fetchArg(id->idval);
	if(ID != NULL)
	{
		return ID->type;
	}
	else if(ID == NULL && g_newFunc != NULL)
	{
		//deprintf("check function argslist\n");
		FieldList p = g_newFunc->tail;
		for(;p != NULL; p = p->tail)
		{
			if(strcmp(p->name,id->idval) == 0)
				return p->type;
		}
		printf("Error type 1 at line %d: Undefined variable '%s'\n",id->lineno,id->idval);
	}
	else
	{
		printf("Error type 1 at line %d: Undefined variable '%s'\n",id->lineno,id->idval);
	}
	return NULL;
}


