#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../header/intercode.h"
#include "../header/debug.h"
#include "../header/treenode.h"
#include "../header/symbol.h"
#include "show.c"


/*
 * intercode.c
 * Function declaration
 */
void init();
struct Var_info* fetchVarInfo(char *name);
struct Var_info* new_VarInfo(char *name,int var_no,int type,int size);

void printVarInfoList();
void insertVarInfo(struct Var_info *new_var);
void insertOperand(Operand op1);

FieldList fetchPara(char *name);
int computeStructSize(FieldList var);

Operand new_Var();
Operand new_Temp();
Operand new_Label();
Operand new_Operand();
InterCode new_InterCode();
OperandNode new_OperandNode();
InterCodeNode new_InterCodeNode();

char* relopOpposite(char *rel);

InterCodeNode translate_Def(treenode *Def_n);
InterCodeNode translate_Dec(treenode * Dec_n);
InterCodeNode translate_Stmt(treenode *Stmt_n);
InterCodeNode translate_Program(treenode *root);
InterCodeNode translate_Args(treenode *Args_n);
InterCodeNode translate_ExtDef(treenode *ExtDef_n);
InterCodeNode translate_FunDec(treenode *FunDec_n);
InterCodeNode translate_CompSt(treenode *CompSt_n);
InterCodeNode translate_DefList(treenode *DefList_n);
InterCodeNode translate_DecList(treenode *DecList_n);
InterCodeNode translate_StmtList(treenode *StmtList_n);
InterCodeNode translate_ExtDefList(treenode *ExtDefList_n);
InterCodeNode translate_ExtDecList(treenode *ExtDecList_n);
InterCodeNode translate_Exp(treenode *Exp_n, Operand place);
InterCodeNode translate_VarDec(treenode *VarDec_n, Operand place);
InterCodeNode translate_ExpDotExp(treenode *enode, Operand place);
InterCodeNode translate_Cond(treenode *cond,Operand label_t,Operand label_f);

InterCodeNode produceNodeByCode(InterCode code);
InterCodeNode produceNodeByNode(InterCodeNode node1, InterCodeNode node2);


/*
 * Gloabal Variable
 */
int g_var_count = 1;
int g_temp_count = 0;
int g_label_count = 0;
FieldList g_func = NULL;		//存储函数
OperandNode arg_list = NULL;	//


int addr = 0;

/*
 * Variable list
 */
struct Var_info *head = NULL;

char* relopOpposite(char *rel){
	if(strcmp(rel,">") == 0)
		return "<=";
	if(strcmp(rel,"<") == 0)
		return ">=";
	if(strcmp(rel,"<=") == 0)
		return ">";
	if(strcmp(rel,">=") == 0)
		return "<";
	if(strcmp(rel,"==") == 0)
		return "!=";
	if(strcmp(rel,"!=") == 0)
		return "==";
}

void insertVarInfo(struct Var_info *new_var){
	if(head == NULL){
		new_var->next = NULL;
		head = new_var;
	}else{
		new_var->next = head;
		head = new_var;
	}
}

void printVarInfoList(){
	struct Var_info *p = head;
	//printf("**** Var_info List ****\n");
	while(p != NULL){
		//printf("\t name:%s var_no:%d\n",p->name,p->var_no);
		p = p->next;
	}
}

struct Var_info* fetchVarInfo(char *name){
	struct Var_info *p = head;
	while(p != NULL){
		if(strcmp(p->name,name) == 0)
			return p;
		p = p->next;
	}
	return NULL;
}

FieldList fetchPara(char *name){
	if(g_func == NULL){
	//	printf("**** g_func is NULL ****\n");
		return NULL;
	}
	FieldList para = g_func->tail;
	while(para != NULL){
		if(strcmp(para->name,name) == 0)
			return para;
		para = para->tail;
	}
	return NULL;
}

struct Var_info* new_VarInfo(char *name,int var_no,int type,int size){
	struct Var_info *var = (struct Var_info*)malloc(sizeof(struct Var_info));
	var->name = name;
	var->var_no = var_no;
	var->type = type;
	var->size = size;
	return var;
}

void init(){
	/*
	 *插入read(),write()函数
	 */
	char *read_name = (char*)malloc(20);
	char *write_name = (char*)malloc(20);
	char *field_name = (char*)malloc(20);
	strcpy(read_name,"read");
	strcpy(write_name,"write");
	strcpy(field_name,"a");
	Type read_type = (Type)malloc(sizeof(struct Type_));
	read_type->kind = basic;
	read_type->u.basic = 0;
	FieldList read_func = (FieldList)malloc(sizeof(struct FieldList_));
	read_func->name = read_name;
	read_func->type = read_type;
	read_func->tail = NULL;
	insertFunc(read_func);

	Type write_type = (Type)malloc(sizeof(struct Type_));
	write_type->kind = basic;
	write_type->u.basic = 0;
	Type field_type = (Type)malloc(sizeof(struct Type_));
	field_type->kind = basic;
	field_type->u.basic = 0;
	FieldList write_func = (FieldList)malloc(sizeof(struct FieldList_));
	FieldList write_field = (FieldList)malloc(sizeof(struct FieldList_));
	write_field->name = field_name;
	write_field->type = field_type;
	write_field->tail = NULL;
	write_func->name = write_name;
	write_func->type = write_type;
	write_func->tail = write_field;
	insertFunc(write_func);
}

int computeStructSize(FieldList var){
	FieldList fp = var->tail;
	int size = 0;
	int array_size = 0;
	int struct_size = 0;
	while(fp != NULL){
		switch(fp->type->kind){
			case basic:
				size += 4;
				break;
			case array:
				array_size = (fp->type->u).array.size;
				size += array_size*4;
				break;
			case structure:
				struct_size = computeStructSize((fp->type->u).structure);
				size += struct_size;
				break;
		}
		fp = fp->tail;
	}
	return size;
}

InterCodeNode produceNodeByNode(InterCodeNode node1, InterCodeNode node2){
	if(node1 == NULL)
		return node2;
	if(node2 == NULL)
		return node1;
	else{
		InterCodeNode p = node1;
		while(p->next != NULL)
			p = p->next;
		p->next = node2;
		node2->prev = p;
		return node1;
	}
}

InterCodeNode produceNodeByCode(InterCode code){
	if(code == NULL){
		//deprintf("code is NULL\n");
		return NULL;
	}
	InterCodeNode new_node = new_InterCodeNode();
	new_node->next = NULL;
	new_node->prev = NULL;
	new_node->code = code;
	return new_node;
}

InterCode new_InterCode(){
	return (InterCode)malloc(sizeof(struct InterCode_));
}

InterCodeNode new_InterCodeNode(){
	return (InterCodeNode)malloc(sizeof(struct InterCodeNode_));
}

OperandNode new_OperandNode(){
	return (OperandNode)malloc(sizeof(struct OperandNode_));
}

void insertOperand(Operand op1){
	OperandNode new_node = new_OperandNode();
	new_node->op = op1;
	new_node->next = NULL;
	if(arg_list == NULL){
		arg_list = new_node;
	}else{
		new_node->next = arg_list;
		arg_list = new_node;
	}
}

Operand new_Temp(){
	Operand temp = (Operand)malloc(sizeof(struct Operand_));
	temp->kind = TEMP;
	temp->u.temp_no = ++g_temp_count;
	return temp;
}

Operand new_Label(){
	Operand label = (Operand)malloc(sizeof(struct Operand_));
	label->kind = LABEL_OP;
	label->u.label_no = ++g_label_count;
	return label;
}

Operand new_Var(){
	Operand var = (Operand)malloc(sizeof(struct Operand_));
	var->kind = VARIABLE;
	var->u.var_no = ++g_var_count;
	return var;
}

Operand new_Operand(){
	return (Operand)malloc(sizeof(struct Operand_));	
}

InterCodeNode translate_Program(treenode *root){
	if(root == NULL){
		//deprintf("Root is NULL\n");
		return NULL;
	}else{
		//program -> extdeflist
		translate_ExtDefList(root->node[0]);
	}
}


InterCodeNode translate_ExtDefList(treenode *ExtDefList_n){
	if(ExtDefList_n == NULL){
		//deprintf("ExtDefList is NULL\n");
		return NULL;
	}
	if(ExtDefList_n->child_num != 2){
		//extdeflist -> empty
		//deprintf("ExtDefList is empty\n");
		return NULL;
	}else{
		//extdeflist -> extdef extdeflist 
		InterCodeNode node1 = translate_ExtDef(ExtDefList_n->node[0]);
		InterCodeNode node2 = translate_ExtDefList(ExtDefList_n->node[1]);
		return produceNodeByNode(node1,node2);
	}
}

InterCodeNode translate_ExtDef(treenode *ExtDef_n){
	if(ExtDef_n == NULL){
		//deprintf("ExtDef is NULL\n");
		return NULL;
	}
	if(ExtDef_n->child_num == 3 && strcmp(ExtDef_n->node[1]->name,"FunDec") == 0){
		//extdef -> specifier fundec compst
		InterCodeNode node1 = translate_FunDec(ExtDef_n->node[1]);
		InterCodeNode node2 = translate_CompSt(ExtDef_n->node[2]);
		return produceNodeByNode(node1,node2);
	}else if(ExtDef_n->child_num == 3 && strcmp(ExtDef_n->node[1]->name,"ExtDecList") == 0){
		//extdef -> specifier extdeclist ;
		return translate_ExtDecList(ExtDef_n->node[1]);
	}
	return NULL;	
}

InterCodeNode translate_ExtDecList(treenode *ExtDecList_n){
	if(ExtDecList_n == NULL){
		//deprintf("ExtDecList is NULL");
		return NULL;
	}
	if(ExtDecList_n->child_num == 1){
		//extdeclist -> vardec
		Operand temp = new_Operand();
		return translate_VarDec(ExtDecList_n->node[0],temp);
	}
	if(ExtDecList_n->child_num == 3){
		Operand temp = new_Operand();
		treenode *vardec_node = ExtDecList_n->node[0];
		treenode *extdeclist_node = ExtDecList_n->node[2];
		InterCodeNode node1 = translate_VarDec(vardec_node,temp);
		InterCodeNode node2 = translate_ExtDecList(extdeclist_node);
		return produceNodeByNode(node1,node2);
	}
}

InterCodeNode translate_FunDec(treenode *FunDec_n){
	if(FunDec_n == NULL){
		//deprintf("FunDec is NULL\n");
		return NULL;
	}
	treenode *ID_node = FunDec_n->node[0];
	char *name = ID_node->idval;
	g_func = (FieldList)fetchFunc(name);
	if(FunDec_n->child_num == 3){
		//fundec -> id ()
		InterCode func_code = new_InterCode();
		Operand func_op = new_Operand();
		func_code->kind = FUNCTION;
		func_code->u.function.func = func_op;
		func_op->kind = FUNC;
		func_op->u.func_name = name;
		return produceNodeByCode(func_code);
	}
	if(FunDec_n->child_num == 4){
		//fundec -> id ( varlist )
		InterCode func_code = new_InterCode();
		Operand func_op = new_Operand();
		func_code->kind = FUNCTION;
		func_code->u.function.func = func_op;
		func_op->kind = FUNC;
		func_op->u.func_name = name;
		InterCodeNode node1 = produceNodeByCode(func_code);
		FieldList p = g_func->tail;
		while(p != NULL){
			if(p->type->kind == array){
				printf("Can not translate the code: Contain multidimensional array and function parameters of array type!\n");
				exit(1);
			}else if(p->type->kind == basic){
				struct Var_info *new_var = new_VarInfo(p->name,++g_var_count,BASIC,4);
				insertVarInfo(new_var);
			}else{
				struct Var_info *new_var = new_VarInfo(p->name,++g_var_count,STRUCT,4);
				insertVarInfo(new_var);
			}
			InterCode param_code = new_InterCode();
			Operand param_op = new_Operand();
			param_code->kind = PARAM;
			param_code->u.param.var = param_op;
			param_op->kind = VARIABLE;
			param_op->u.var_no = g_var_count;
			InterCodeNode node2 = produceNodeByCode(param_code);
			produceNodeByNode(node1,node2);
			p = p->tail;
		}
		return node1;
	}
}

InterCodeNode translate_CompSt(treenode *CompSt_n){
	if(CompSt_n == NULL){
		//deprintf("CompSt is NULL\n");
		return NULL;
	}
	//compst -> { deflist stmtlist }
	assert(CompSt_n->child_num == 4);
	InterCodeNode node1 = translate_DefList(CompSt_n->node[1]);
	InterCodeNode node2 = translate_StmtList(CompSt_n->node[2]);
	return produceNodeByNode(node1,node2);
}
InterCodeNode translate_DefList(treenode *DefList_n){
	if(DefList_n == NULL){
		//deprintf("DefList is NULL\n");
		return NULL;
	}
	if(DefList_n->child_num != 2){
		// deflist -> empty
		//deprintf("DefList is empty\n");
		return NULL;
	}
	//deflist -> def deflist
	InterCodeNode node1 = translate_Def(DefList_n->node[0]);
	InterCodeNode node2 = translate_DefList(DefList_n->node[1]);
	return produceNodeByNode(node1,node2);
}

InterCodeNode translate_StmtList(treenode *StmtList_n){
	if(StmtList_n == NULL){
		//deprintf("StmtList is NULL\n");
		return NULL;
	}
	if(StmtList_n->child_num != 2){
		//stmtlist -> empty
		assert(StmtList_n->child_num == 0);
		//deprintf("StmtList is empty\n");
		return NULL;
	}
	//stmtlist -> stmt stmtlist
	InterCodeNode node1 = translate_Stmt(StmtList_n->node[0]);
	//show(node1);
	InterCodeNode node2 = translate_StmtList(StmtList_n->node[1]);
	return produceNodeByNode(node1,node2);
}

InterCodeNode translate_Def(treenode *Def_n){
	if(Def_n == NULL){
		//deprintf("Def is NULL\n");
		return NULL;
	}
	// def -> specifier declist ;
	assert(Def_n->child_num == 3);
	return translate_DecList(Def_n->node[1]);
}

InterCodeNode translate_DecList(treenode *DecList_n){
	if(DecList_n == NULL){
		//deprintf("DecList is NULL\n");
		return NULL;
	}
	if(DecList_n->child_num == 1){
		//declist -> dec
		return translate_Dec(DecList_n->node[0]);
	}
	//declist -> dec , declist
	assert(DecList_n->child_num == 3);
	InterCodeNode node1 = translate_Dec(DecList_n->node[0]);
	InterCodeNode node2 = translate_DecList(DecList_n->node[2]);
	return produceNodeByNode(node1,node2);
}

InterCodeNode translate_Dec(treenode * Dec_n){
	if(Dec_n == NULL){
		//deprintf("Dec is NULL\n");
		return NULL;
	}
	if(Dec_n->child_num == 1){
		//dec -> vardec
		Operand temp = new_Operand();
		return translate_VarDec(Dec_n->node[0],temp);
	}
	//dec -> vardec = exp
	assert(Dec_n->child_num == 3);
	Operand left = new_Operand();
	Operand right = new_Temp();
	InterCodeNode node1 = translate_VarDec(Dec_n->node[0],left);
	InterCodeNode node2 = translate_Exp(Dec_n->node[2],right);
	InterCode assign_code = new_InterCode();
	assign_code->kind = ASSIGN;
	assign_code->u.assign.left = left;
	assign_code->u.assign.right = right;
	InterCodeNode node3 = produceNodeByCode(assign_code);
	InterCodeNode temp = produceNodeByNode(node1,node2);
	return produceNodeByNode(temp,node3);
}

InterCodeNode translate_VarDec(treenode *VarDec_n, Operand place){
	if(VarDec_n == NULL){
		//deprintf("VarDec is NULL\n");
		return NULL;
	}
	treenode *id = VarDec_n->node[0];
	char *name = id->idval;
	if(VarDec_n->child_num == 1){
		// vardec -> id
		FieldList var = fetchArg(name);
		if(var != NULL){
			if(var->type->kind == basic){
				struct Var_info *new_var = new_VarInfo(name,++g_var_count,BASIC,4);
				insertVarInfo(new_var);

				place->kind = VARIABLE;
				place->u.var_no = g_var_count;
				return NULL;
			}else{
				assert(var->type->kind == structure);
				int size = computeStructSize((var->type->u).structure);
				struct Var_info *new_var = new_VarInfo(name,++g_var_count,STRUCT,size);
				insertVarInfo(new_var);

				Operand var_op = new_Operand();
				Operand size_op = new_Operand();
				var_op->kind = VARIABLE;
				var_op->u.var_no = g_var_count;
				size_op->kind = SIZE;
				size_op->u.value = size;
				InterCode dec_code = new_InterCode();
				dec_code->kind = DEC;
				dec_code->u.dec.var = var_op;
				dec_code->u.dec.size = size_op;
				return produceNodeByCode(dec_code);
			}
		}
	}
	assert(VarDec_n->child_num == 4);
	//vardec -> vardec [int]
	//vardec -> id
	treenode *num_node = VarDec_n->node[2];
	treenode *check_multi = VarDec_n->node[0];
	int size = num_node->ival;
	if(check_multi->child_num != 1){
		printf("Can not translate the code: contain multidimensional array and function parameters of array type\n");
		exit(1);
	}
	treenode *array_id = check_multi->node[0];
	FieldList array_var = fetchArg(array_id->idval);
	if(array_var != NULL){
		int elem = 4;
		Type base_type = (array_var->type->u).array.elem;
		if(base_type->kind == basic){
			struct Var_info *new_var = new_VarInfo(array_var->name,++g_var_count,ARRAY,size*elem);
			insertVarInfo(new_var);
		}else if(base_type->kind == structure){
			elem = computeStructSize(base_type->u.structure);
			struct Var_info *new_var = new_VarInfo(array_var->name,++g_var_count,STRUCT,size*elem);
			insertVarInfo(new_var);
		}

		Operand var_op = new_Operand();
		Operand size_op = new_Operand();
		size_op->kind = SIZE;
		size_op->u.value = size*elem;
		var_op->kind = VARIABLE;
		var_op->u.var_no = g_var_count;
		//array size dec
		InterCode dec_code = new_InterCode();
		dec_code->kind = DEC;
		dec_code->u.dec.var = var_op;
		dec_code->u.dec.size = size_op;
		return produceNodeByCode(dec_code);
	}
	//deprintf("Error NULL!\n");
	return NULL;
}

InterCodeNode translate_Exp(treenode *Exp_n, Operand place){
	if(Exp_n == NULL){
		//deprintf("Exp is NULL\n");
		return NULL;
	}
	switch(Exp_n->child_num){

		case 1:{
			treenode *node = Exp_n->node[0];
			if(node->type == NODE_INT){
				//exp -> int
				int value = node->ival;
				place->kind = CONSTANT;
				place->u.value = value;
				return NULL;
			}else if(node->type == NODE_FLOAT){
				//exp -> float
				//printf("******* No Float Type *******\n");
				//exit(1);
				return NULL;
			}else{ 
				//exp -> id
				treenode *id = Exp_n->node[0];
				char *name = id->idval;
				struct Var_info *var = fetchVarInfo(name);
				if(var != NULL){
					place->u.var_no = var->var_no;
					if(var->type == BASIC || var->type == ARRAY){
						place->kind = VARIABLE;
					}else if(var->type == STRUCT){
						place->kind = REFERENCE;
					}
				}else{
					//deprintf("**** can not find var in VarInfo ****\n");
				}
				return NULL;
			}
			break;
		}
		case 2:{
			treenode *op = Exp_n->node[0];
			treenode *exp = Exp_n->node[1];
			if(strcmp(op->operval,"MINUS") == 0){
				//MINUS exp
				//- exp
				Operand temp = new_Operand();
				InterCodeNode node1 = translate_Exp(exp,temp);
				if(temp->kind == CONSTANT){
					place->kind = CONSTANT;
					place->u.value = -(temp->u.value);
					return NULL;
				}else{
					Operand zero = new_Operand();
					zero->kind = CONSTANT;
					zero->u.value = 0;
					InterCode sub_code = new_InterCode();
					sub_code->kind = SUB;
					sub_code->u.binop.result = place;
					sub_code->u.binop.op1 = zero;
					sub_code->u.binop.op2 = temp;
					InterCodeNode node2 = produceNodeByCode(sub_code);
					return produceNodeByNode(node1,node2);
				}
			}else if(strcmp(op->operval,"NOT") == 0){
				//NOT exp
				//! exp
				Operand label1 = new_Label();
				Operand label2 = new_Label();
				Operand zero = new_Operand();
				zero->kind = CONSTANT;
				zero->u.value = 0;
				Operand one  = new_Operand();
				one->kind = CONSTANT;
				one->u.value = 1;

				InterCode code0 = new_InterCode();
				code0->kind = ASSIGN;
				code0->u.assign.left = place;
				code0->u.assign.right = zero;
				InterCodeNode node0 = produceNodeByCode(code0);

				InterCodeNode node1 = translate_Cond(Exp_n,label1,label2);
				
				InterCode code2 = new_InterCode();
				code2->kind = LABEL;
				code2->u.label.label_op = label1;
				InterCodeNode node2 = NULL;//produceNodeByCode(code2);

				InterCode code3 =  new_InterCode();
				code3->kind = ASSIGN;
				code3->u.assign.left = place;
				code3->u.assign.right = one;
				InterCodeNode node3 =  produceNodeByCode(code3);

				InterCode code4 = new_InterCode();
				code4->kind = LABEL;
				code4->u.label.label_op = label2;
				InterCodeNode node4 = produceNodeByCode(code4);

				InterCodeNode temp1 = produceNodeByNode(node0,node1);
				InterCodeNode temp2 = produceNodeByNode(temp1,node2);
				InterCodeNode temp3 = produceNodeByNode(temp2,node3);
				return produceNodeByNode(temp3,node4);
			}else{
				return NULL;
			}
			break;
		}
		case 3:{
			treenode *prev = Exp_n->node[0];
			treenode *midd = Exp_n->node[1];
			treenode *next = Exp_n->node[2];
			if(strcmp(midd->operval,"ASSIGNOP") == 0){
				//exp = exp
				Operand left = new_Operand();
				Operand right = new_Temp();
				InterCodeNode node1 = translate_Exp(prev,left);
				InterCodeNode node2 = translate_Exp(next,right);
				//show(node1);
				//printf("**** split node ****\n");
				//show(node2);
				InterCode assign_code = new_InterCode();
				assign_code->kind = ASSIGN;
				assign_code->u.assign.left = left;
				assign_code->u.assign.right = right;
				place = left;
				InterCodeNode node3 = produceNodeByCode(assign_code);
				//printf("**** show node3 ****\n");
				//show(node3);
				InterCodeNode temp = produceNodeByNode(node1,node2);
				return produceNodeByNode(temp,node3);
			}else if(strcmp(midd->operval,"PLUS") == 0){
				//exp + exp
				Operand temp1 = new_Temp();
				Operand temp2 = new_Temp();
				InterCodeNode node1 = translate_Exp(prev,temp1);
				InterCodeNode node2 = translate_Exp(next,temp2);
				if(temp1->kind == CONSTANT && temp2->kind == CONSTANT){
					place->kind = CONSTANT;
					place->u.value = temp1->u.value + temp2->u.value;
					return NULL;
				}
				InterCode plus_code = new_InterCode();
				plus_code->kind = ADD;
				plus_code->u.binop.result = place;
				plus_code->u.binop.op1 = temp1;
				plus_code->u.binop.op2 = temp2;
				InterCodeNode node3 = produceNodeByCode(plus_code);
				InterCodeNode temp = produceNodeByNode(node1,node2);
				return produceNodeByNode(temp,node3);
			}else if(strcmp(midd->operval,"MINUS") == 0){
				//exp - exp
				Operand temp1 = new_Temp();
				Operand temp2 = new_Temp();
				InterCodeNode node1 = translate_Exp(prev,temp1);
				InterCodeNode node2 = translate_Exp(next,temp2);
				if(temp1->kind == CONSTANT && temp2->kind == CONSTANT){
					place->kind = CONSTANT;
					place->u.value = temp1->u.value - temp2->u.value;
					return NULL;
				}
				// if(temp1->kind == temp2->kind && temp1->kind == VARIABLE){
				// 	if(temp1->u.var_no == temp2->u.var_no){
				// 		place->kind = CONSTANT;
				// 		place->u.value = 0;
				// 	}
				// }
				InterCode minus_code = new_InterCode();
				minus_code->kind = SUB;
				minus_code->u.binop.result = place;
				minus_code->u.binop.op1 = temp1;
				minus_code->u.binop.op2 = temp2;
				InterCodeNode node3 = produceNodeByCode(minus_code);
				InterCodeNode temp = produceNodeByNode(node1,node2);
				return produceNodeByNode(temp,node3);
			}else if(strcmp(midd->operval,"STAR") == 0){
				//exp * exp
				Operand temp1 = new_Temp();
				Operand temp2 = new_Temp();
				InterCodeNode node1 = translate_Exp(prev,temp1);
				InterCodeNode node2 = translate_Exp(next,temp2);
				if(temp1->kind == CONSTANT && temp2->kind == CONSTANT){
					place->kind = CONSTANT;
					place->u.value = temp1->u.value * temp2->u.value;
					return NULL;
				}
				InterCode star_code = new_InterCode();
				star_code->kind = MUL;
				star_code->u.binop.result = place;
				star_code->u.binop.op1 = temp1;
				star_code->u.binop.op2 = temp2;
				InterCodeNode node3 = produceNodeByCode(star_code);
				InterCodeNode temp = produceNodeByNode(node1,node2);
				return produceNodeByNode(temp,node3);
			}else if(strcmp(midd->operval,"DIV") == 0){
				//exp / exp
				Operand temp1 = new_Temp();
				Operand temp2 = new_Temp();
				InterCodeNode node1 = translate_Exp(prev,temp1);
				InterCodeNode node2 = translate_Exp(next,temp2);
				if(temp1->kind == CONSTANT && temp2->kind == CONSTANT){
					place->kind = CONSTANT;
					place->u.value = (temp1->u.value) / (temp2->u.value);
					return NULL;
				}
				InterCode div_code = new_InterCode();
				div_code->kind = DIV;
				div_code->u.binop.result = place;
				div_code->u.binop.op1 = temp1;
				div_code->u.binop.op2 = temp2;
				InterCodeNode node3 = produceNodeByCode(div_code);
				InterCodeNode temp = produceNodeByNode(node1,node2);
				return produceNodeByNode(temp,node3);
			}else if(strcmp(midd->operval,"relop") == 0 
					|| strcmp(midd->operval,"AND") == 0
					|| strcmp(midd->operval,"OR") == 0){
				//exp relop exp	
				//exp or exp
				//exp and exp
				Operand label1 = new_Label();
				Operand label2 = new_Label();
				Operand zero = new_Operand();
				zero->kind = CONSTANT;
				zero->u.value = 0;
				Operand one  = new_Operand();
				one->kind = CONSTANT;
				one->u.value = 1;

				InterCode code0 = new_InterCode();
				code0->kind = ASSIGN;
				code0->u.assign.left = place;
				code0->u.assign.right = zero;
				InterCodeNode node0 = produceNodeByCode(code0);

				InterCodeNode node1 = translate_Cond(Exp_n,label1,label2);
				
				InterCode code2 = new_InterCode();
				code2->kind = LABEL;
				code2->u.label.label_op = label1;
				InterCodeNode node2 = NULL;//produceNodeByCode(code2);

				InterCode code3 =  new_InterCode();
				code3->kind = ASSIGN;
				code3->u.assign.left = place;
				code3->u.assign.right = one;
				InterCodeNode node3 =  produceNodeByCode(code3);

				InterCode code4 = new_InterCode();
				code4->kind = LABEL;
				code4->u.label.label_op = label2;
				InterCodeNode node4 = produceNodeByCode(code4);

				InterCodeNode temp1 = produceNodeByNode(node0,node1);
				InterCodeNode temp2 = produceNodeByNode(temp1,node2);
				InterCodeNode temp3 = produceNodeByNode(temp2,node3);
				return produceNodeByNode(temp3,node4);

			}else if(strcmp(prev->operval,"LP") == 0 && strcmp(next->operval,"RP") == 0){
				//( exp )
				return translate_Exp(midd,place);
			}else if(prev->type == NODE_ID && strcmp(next->operval,"RP") == 0){
				//id ()
				FieldList func = fetchFunc(prev->idval);
				if(func == NULL){
					//deprintf("Can not find the function!\n");
					return NULL;
				}else{
					if(strcmp(func->name,"read") == 0){
						InterCode read_code = new_InterCode();
						read_code->kind = READ;
						read_code->u.read.arg = place;
						return produceNodeByCode(read_code);
					}else{
						Operand func_op = new_Operand();
						func_op->kind = FUNC;
						func_op->u.func_name = func->name;
						InterCode call_code = new_InterCode();
						call_code->kind = CALL;
						call_code->u.call.result = place;
						call_code->u.call.function = func_op;
						return produceNodeByCode(call_code);
					}
				}
			}else if(strcmp(midd->operval,"DOT") == 0){
				//Exp . id
				treenode *main_id = Exp_n->node[0]->node[0];
				treenode *sub_id = Exp_n->node[2];
				FieldList if_para = fetchPara(main_id->idval);
				FieldList para_field;
				if(if_para != NULL){
					//in parameters 
					para_field = (if_para->type->u).structure->tail;
					struct Var_info *var = fetchVarInfo(main_id->idval);
					if(var == NULL){
						//printf("Can not find the Para\n");
						return NULL;
					}
					int v_no = var->var_no;
					Operand temp_op = new_Operand();
					temp_op->kind = ADDRESS;
					temp_op->u.var_no = v_no;

					if(strcmp(sub_id->idval,para_field->name) == 0){
						//first field
						Operand temp = new_Temp();
						place->kind = TEMP;
						place->u.temp_no = temp->u.temp_no;
						
						InterCode addr_code = new_InterCode();
						addr_code->kind = ASSIGN;
						addr_code->u.assign.left = temp;
						addr_code->u.assign.right = temp_op;
						return produceNodeByCode(addr_code);
					}else{
						int count = 0;
						while(para_field != NULL){
							if(strcmp(sub_id->idval,para_field->name) == 0)
								break;
							count++;
							para_field = para_field->tail;
						}
						Operand res_op = new_Temp();
						Operand op1 = new_Operand();
						op1->kind = VARIABLE;
						op1->u.var_no = v_no;
						Operand op2 = new_Operand();
						op2->kind = CONSTANT;
						op2->u.value = count*4;
						
						InterCode add_code = new_InterCode();
						add_code->kind = ADD;
						add_code->u.binop.result = res_op;
						add_code->u.binop.op1 = op1;
						add_code->u.binop.op2 = op2;
						InterCodeNode node1 = produceNodeByCode(add_code);

						Operand get_value = new_Operand();
						get_value->kind = ADDRESS;
						get_value->u.temp_no = res_op->u.temp_no;
						Operand res1_op = new_Temp();

						InterCode assign_code = new_InterCode();
						assign_code->kind = ASSIGN;
						assign_code->u.assign.left = res1_op;
						assign_code->u.assign.right = get_value;
						InterCodeNode node2 = produceNodeByCode(assign_code);

						place->kind = TEMP;
						place->u.temp_no = res1_op->u.temp_no;

						return produceNodeByNode(node1,node2);
					}
				}else{
					//not in parameters
					struct Var_info *var = fetchVarInfo(main_id->idval);
					if(var == NULL){
						//printf("Can not find the Var_Info\n");
						return NULL;
					}
					int v_no = var->var_no;
					Operand temp_op = new_Operand();
					temp_op->kind = REFERENCE;
					temp_op->u.var_no = v_no;
					FieldList normal_var = fetchArg(main_id->idval);
					FieldList type_field = (normal_var->type->u).structure->tail;
					if(strcmp(sub_id->idval,type_field->name) == 0){
						//first field
						Operand temp = new_Temp();
						place->kind = ADDRESS;
						place->u.temp_no = temp->u.temp_no;
						
						InterCode addr_code = new_InterCode();
						addr_code->kind = ASSIGN;
						addr_code->u.assign.left = temp;
						addr_code->u.assign.right = temp_op;
						return produceNodeByCode(addr_code);
					}else{
						int count = 0;
						while(type_field != NULL){
							if(strcmp(sub_id->idval,type_field->name) == 0)
								break;
							count++;
							type_field = type_field->tail;
						}
						Operand res_op = new_Temp();
						Operand op2 = new_Operand();
						op2->kind = CONSTANT;
						op2->u.value = count*4;
						
						InterCode add_code = new_InterCode();
						add_code->kind = ADD;
						add_code->u.binop.result = res_op;
						add_code->u.binop.op1 = temp_op;
						add_code->u.binop.op2 = op2;
						InterCodeNode node1 = produceNodeByCode(add_code);

						place->kind = ADDRESS;
						place->u.temp_no = res_op->u.temp_no;
						return node1;
					}
				}
			}
			break;
		}
		case 4:{
			if(strcmp(Exp_n->node[2]->name,"Args") == 0){
				//id (args)
				treenode *id = Exp_n->node[0];
				FieldList func = fetchFunc(id->idval);
				if(func == NULL){
					//printf("**** Can not find the function ****\n");
					return NULL;
				}
				arg_list = NULL;
				InterCodeNode node1 = translate_Args(Exp_n->node[2]);
				if(strcmp(func->name,"write") == 0){
					InterCode write_code = new_InterCode();
					write_code->kind = WRITE;
					write_code->u.write.arg = arg_list->op;
					InterCodeNode node2 = produceNodeByCode(write_code);
					return produceNodeByNode(node1,node2);
				}else{
					OperandNode p = arg_list;
					InterCodeNode link_head = new_InterCodeNode();
					while(p != NULL){
						InterCode arg_code = new_InterCode();
						arg_code->kind = ARG;
						arg_code->u.arg.arg_op = p->op;
						InterCodeNode node2 = produceNodeByCode(arg_code);
						//show(node2);
						produceNodeByNode(link_head,node2);
						p = p->next;
					}
					Operand func_op = new_Operand();
					func_op->kind = FUNC;
					func_op->u.func_name = func->name;
					InterCode call_code = new_InterCode();
					call_code->kind = CALL;
					call_code->u.call.result = place;
					call_code->u.call.function = func_op;
					InterCodeNode node3 = produceNodeByCode(call_code);
					InterCodeNode temp = produceNodeByNode(node1,link_head->next);
					return produceNodeByNode(temp,node3);
				}
			}
			if(strcmp(Exp_n->node[2]->name,"Exp") == 0){
				//exp [exp]
				treenode *pre_exp = Exp_n->node[0];
				if(pre_exp->child_num == 3){
					//exp.id [exp] 
					treenode *inner_exp = pre_exp->node[0];
					if(inner_exp->child_num == 1){
						//id.id[exp]
						return translate_ExpDotExp(Exp_n,place);
					}else if(inner_exp->child_num == 4){
						//id[exp].id[exp]
						return NULL;
					}
				}
				treenode *id = pre_exp->node[0];
				treenode *index = Exp_n->node[2];
				FieldList if_para = fetchPara(id->idval);
				if(pre_exp->child_num == 4 || if_para != NULL){
					printf("Can not translate the code: contain multidimensional array and function parameters of array type\n");
					exit(1);
				}else{
					struct Var_info *var = fetchVarInfo(id->idval);
					if(var == NULL){
						return NULL;
					}
					Operand array_op = new_Operand();
					array_op->kind = REFERENCE;
					array_op->u.var_no = var->var_no;
					if(index->node[0]->type == NODE_INT){
						// id [int]
						int size = index->node[0]->ival;
						if(size == 0){
							Operand t1 = new_Temp();
							InterCode assign_code = new_InterCode();
							assign_code->kind = ASSIGN;
							assign_code->u.assign.left = t1;
							assign_code->u.assign.right = array_op;

							place->kind = ADDRESS;
							place->u.temp_no = t1->u.temp_no;
							return produceNodeByCode(assign_code);
						}else{
							Operand res_op = new_Temp();
							Operand offset_num = new_Operand();
							offset_num->kind = CONSTANT;
							offset_num->u.value = size*4;

							InterCode add_code = new_InterCode();
							add_code->kind = ADD;
							add_code->u.binop.result = res_op;
							add_code->u.binop.op1 = array_op;
							add_code->u.binop.op2 = offset_num;

							place->kind = ADDRESS;
							place->u.temp_no = res_op->u.temp_no;
							return produceNodeByCode(add_code);
						}
					}else if(index->node[0]->type == NODE_ID){
						//id [id]
						char *name = index->node[0]->idval;
						struct Var_info *var1 = fetchVarInfo(name);
						Operand res_op = new_Temp();
						Operand op1 = new_Operand();
						op1->kind = VARIABLE;
						op1->u.var_no = var1->var_no;
				
						Operand op2 = new_Operand();
						op2->kind = CONSTANT;
						op2->u.value = 4;

						InterCode mul_code = new_InterCode();
						mul_code->kind = MUL;
						mul_code->u.binop.result = res_op;
						mul_code->u.binop.op1 = op1;
						mul_code->u.binop.op2 = op2;
						InterCodeNode node1 = produceNodeByCode(mul_code);

						Operand res2_op = new_Temp();
						InterCode add_code = new_InterCode();
						add_code->kind = ADD;
						add_code->u.binop.result = res2_op;
						add_code->u.binop.op1 = array_op;
						add_code->u.binop.op2 = res_op;
						InterCodeNode node2 = produceNodeByCode(add_code);

						place->kind = ADDRESS;
						place->u.temp_no = res2_op->u.temp_no;
						return produceNodeByNode(node1,node2);
					}else{
						//exp
						Operand t1 = new_Temp();
						InterCodeNode node1 = translate_Exp(index,t1);

						Operand t2 = new_Operand();
						t2->kind = CONSTANT;
						t2->u.value = 4;

						Operand res_op = new_Temp();
						InterCode mul_code = new_InterCode();
						mul_code->kind = MUL;
						mul_code->u.binop.result = res_op;
						mul_code->u.binop.op1 = t1;
						mul_code->u.binop.op2 = t2;
						InterCodeNode node2 = produceNodeByCode(mul_code);

						Operand res2_op = new_Temp();
						InterCode add_code = new_InterCode();
						add_code->kind = ADD;
						add_code->u.binop.result = res2_op;
						add_code->u.binop.op1 = res_op;
						add_code->u.binop.op2 = array_op;
						InterCodeNode node3 = produceNodeByCode(add_code);

						place->kind = ADDRESS;
						place->u.temp_no = res2_op->u.temp_no;
						InterCodeNode temp = produceNodeByNode(node1,node2);
						return produceNodeByNode(temp,node3);
					}
				}
			}
			break;
		}
	}
}

InterCodeNode translate_Args(treenode *Args_n){
	if(Args_n == NULL){
		//deprintf("Args is NULL\n");
		return NULL;
	}
	if(Args_n->child_num == 1){
		// args -> exp
		Operand t1 = new_Temp();
		InterCodeNode node1 = translate_Exp(Args_n->node[0],t1);
		insertOperand(t1);
		return node1;
	}
	if(Args_n->child_num == 3){
		// args -> exp , args
		Operand t1 = new_Temp();
		InterCodeNode node1 = translate_Exp(Args_n->node[0],t1);
		insertOperand(t1);
		InterCodeNode node2 = translate_Args(Args_n->node[2]);
		return produceNodeByNode(node1,node2);
	}
}

InterCodeNode translate_Cond(treenode *cond,Operand label_t,Operand label_f){
	if(cond == NULL){
		//deprintf("Cond is NULL\n");
		return NULL;
	}
	if(cond->child_num == 2){
		treenode *op = cond->node[0];
		treenode *exp = cond->node[1];
		if(strcmp(op->operval,"NOT") == 0){
			return translate_Cond(exp,label_f,label_t);
		}
	}
	if(cond->child_num == 3){
		treenode *prev = cond->node[0];
		treenode *midd = cond->node[1];
		treenode *next = cond->node[2];
		if(strcmp(midd->operval,"RELOP") == 0){
			//exp relop exp
			Operand t1 = new_Temp();
			Operand t2 = new_Temp();
			InterCodeNode node1 = translate_Exp(prev,t1);
			InterCodeNode node2 = translate_Exp(next,t2);
			
			Operand relop = new_Operand();
			relop->kind = RELOP_OP;
			strcpy(relop->u.relop_name,relopOpposite(midd->name));
			
			InterCode relop_code = new_InterCode();
			relop_code->kind = RELOP;
			relop_code->u.rel.op1 = t1;
			relop_code->u.rel.op2 = t2;
			relop_code->u.rel.relop_op = relop;
			relop_code->u.rel.label_op = label_f;
			InterCodeNode node3 = produceNodeByCode(relop_code);

			/*InterCode gto_code = new_InterCode();
			gto_code->kind = GOTO;
			gto_code->u.gto.label_op = label_f;
			InterCodeNode node4 = produceNodeByCode(gto_code);*/

			InterCodeNode temp1 = produceNodeByNode(node1,node2);
			InterCodeNode temp2 = produceNodeByNode(temp1,node3);
			return temp2;

		}else if(strcmp(midd->operval,"AND") == 0){
			// exp and exp
			Operand label1 = new_Label();
			InterCodeNode node1 = translate_Cond(prev,label1,label_f);
			InterCodeNode node3 = translate_Cond(next,label_t,label_f);
			
			InterCode label_code = new_InterCode();
			label_code->kind = LABEL;
			label_code->u.label.label_op = label1;

			InterCodeNode node2 = produceNodeByCode(label_code);
			InterCodeNode temp = produceNodeByNode(node1,node2);
			return produceNodeByNode(temp,node3);
		
		}else if(strcmp(midd->operval,"OR") == 0){
			// exp or exp
			Operand label1 = new_Label();
			InterCodeNode node1 = translate_Cond(prev,label_t,label1);
			InterCodeNode node3 = translate_Cond(next,label_t,label_f);
			
			InterCode label_code = new_InterCode();
			label_code->kind = LABEL;
			label_code->u.label.label_op = label1;

			InterCodeNode node2 = produceNodeByCode(label_code);
			InterCodeNode temp = produceNodeByNode(node1,node2);
			return produceNodeByNode(temp,node3);
		}
	}
	//other case
	Operand t1 = new_Temp();
	InterCodeNode node1 = translate_Exp(cond,t1);
	
	Operand zero = new_Operand();
	zero->kind = CONSTANT;
	zero->u.value = 0;
	Operand unequal = new_Operand();
	unequal->kind = RELOP_OP;
	strcpy(unequal->u.relop_name,"==");

	InterCode unequal_code = new_InterCode();
	unequal_code->kind = RELOP;
	unequal_code->u.rel.op1 = t1;
	unequal_code->u.rel.relop_op = unequal;
	unequal_code->u.rel.op2 = zero;
	unequal_code->u.rel.label_op = label_f;
	InterCodeNode node2 = produceNodeByCode(unequal_code);

	// InterCode gto_code = new_InterCode();
	// gto_code->kind = GOTO;
	// gto_code->u.gto.label_op = label_f;
	// InterCodeNode node3 = produceNodeByCode(gto_code);

	InterCodeNode temp = produceNodeByNode(node1,node2);
	return temp;
}

InterCodeNode translate_Stmt(treenode *Stmt_n){
	if(Stmt_n == NULL){
		//deprintf("Stmt is NULL\n");
		return NULL;
	}
	if(Stmt_n->child_num == 1){
		//compst
		return translate_CompSt(Stmt_n->node[0]);
	}else if(Stmt_n->child_num == 2){
		//exp ;
		Operand temp = new_Operand();
		return translate_Exp(Stmt_n->node[0],temp);
	}else if(Stmt_n->child_num == 3){
		//return exp ;
		Operand temp = new_Temp();
		InterCodeNode node1 = translate_Exp(Stmt_n->node[1],temp);
		
		InterCode ret_code = new_InterCode();
		ret_code->kind = RETURN;
		ret_code->u.retn.return_op = temp;
		InterCodeNode node2 = produceNodeByCode(ret_code);
		return produceNodeByNode(node1,node2);
	}else if(Stmt_n->child_num == 5){
		treenode *head = Stmt_n->node[0];
		if(strcmp(head->operval,"IF") == 0){
			//if (exp) stmt	
			Operand label1 = new_Label();
			Operand label2 = new_Label();
			InterCodeNode node1 = translate_Cond(Stmt_n->node[2],label1,label2);
			InterCodeNode node3 = translate_Stmt(Stmt_n->node[4]);
			
			InterCode code2 = new_InterCode();
			code2->kind = LABEL;
			code2->u.label.label_op = label1;
			InterCodeNode node2 = NULL;//produceNodeByCode(code2);

			InterCode code4 = new_InterCode();
			code4->kind = LABEL;
			code4->u.label.label_op = label2;
			InterCodeNode node4 = produceNodeByCode(code4);

			InterCodeNode temp1 = produceNodeByNode(node1,node2);
			InterCodeNode temp2 = produceNodeByNode(temp1,node3);
			return produceNodeByNode(temp2,node4);
		}
		if(strcmp(head->operval,"WHILE") == 0){
			//while (exp) stmt
			Operand label1 = new_Label();
			Operand label2 = new_Label();
			Operand label3 = new_Label();

			InterCode code1 = new_InterCode();
			code1->kind = LABEL;
			code1->u.label.label_op = label1;
			InterCodeNode node1 = produceNodeByCode(code1);
			
			InterCodeNode node2 = translate_Cond(Stmt_n->node[2],label2,label3);
			
			InterCode code3 = new_InterCode();
			code3->kind = LABEL;
			code3->u.label.label_op = label2;
			InterCodeNode node3 = NULL;//produceNodeByCode(code3);

			InterCodeNode node4 = translate_Stmt(Stmt_n->node[4]);

			InterCode code5 = new_InterCode();
			code5->kind = GOTO;
			code5->u.label.label_op = label1;
			InterCodeNode node5 = produceNodeByCode(code5);

			InterCode code6 = new_InterCode();
			code6->kind = LABEL;
			code6->u.label.label_op = label3;
			InterCodeNode node6 = produceNodeByCode(code6);

			InterCodeNode temp1 = produceNodeByNode(node1,node2);
			InterCodeNode temp2 = produceNodeByNode(temp1,node3);
			InterCodeNode temp3 = produceNodeByNode(temp2,node4);
			InterCodeNode temp4 = produceNodeByNode(temp3,node5);
			return produceNodeByNode(temp4,node6);
		}
	}else if(Stmt_n->child_num == 7){
		//if (exp) stmt else stmt
		Operand label1 = new_Label();
		Operand label2 = new_Label();
		Operand label3 = new_Label();

		InterCodeNode node1 = translate_Cond(Stmt_n->node[2],label1,label2);

		InterCode code2 = new_InterCode();
		code2->kind = LABEL;
		code2->u.label.label_op = label1;
		InterCodeNode node2 = NULL;//produceNodeByCode(code2);

		InterCodeNode node3 = translate_Stmt(Stmt_n->node[4]);

		InterCode code4 = new_InterCode();
		code4->kind = GOTO;
		code4->u.label.label_op = label3;
		InterCodeNode node4 = produceNodeByCode(code4);

		InterCode code5 = new_InterCode();
		code5->kind = LABEL;
		code5->u.label.label_op = label2;
		InterCodeNode node5 = produceNodeByCode(code5);

		InterCodeNode node6 = translate_Stmt(Stmt_n->node[6]);

		InterCode code7 = new_InterCode();
		code7->kind = LABEL;
		code7->u.label.label_op = label3;
		InterCodeNode node7 = produceNodeByCode(code7);

		InterCodeNode temp1 = produceNodeByNode(node1,node2);
		InterCodeNode temp2 = produceNodeByNode(temp1,node3);
		InterCodeNode temp3 = produceNodeByNode(temp2,node4);
		InterCodeNode temp4 = produceNodeByNode(temp3,node5);
		InterCodeNode temp5 = produceNodeByNode(temp4,node6);
		return produceNodeByNode(temp5,node7);
	}else{
		//deprintf("**** Illegal Stmt Error ****\n");
		return NULL;
	}
}

//maybe useless
InterCodeNode translate_ExpDotExp(treenode *enode, Operand place){
	//Exp -> Exp [Exp]
	//		Exp -> Exp.id
	//
	//		Exp -> Exp[Exp].id[Exp]
	treenode *exp1 = enode->node[0];
	treenode *index_exp = enode->node[2];

	treenode *exp2 = exp1->node[0];
	treenode *field_id = exp1->node[2];

	treenode *var_id = exp2->node[0];

	Operand start = new_Operand();

	FieldList if_para = fetchPara(var_id->idval);
	FieldList field = NULL;
	if(if_para != NULL){
		struct Var_info *temp_var = fetchVarInfo(var_id->idval);
		start->kind = VARIABLE;
		start->u.var_no = temp_var->var_no;
		field = (if_para->type->u).structure->tail;
	}else{
		struct Var_info *temp_var = fetchVarInfo(var_id->idval);
		start->kind = REFERENCE;
		start->u.var_no = temp_var->var_no;
		FieldList temp_var2 = fetchArg(var_id->idval);
		field = (temp_var2->type->u).structure->tail;
	}
	int count = 0;
	FieldList p = field;
	while(p != NULL){
		if(strcmp(p->name,field_id->idval) == 0)
			break;
		switch(p->type->kind){
			case basic:
				count += 4;break;
			case array:
				count += 4*((p->type->u).array.size);break;
			case structure:
				count += computeStructSize((p->type->u).structure);
				break;
		}
		p = p->tail;
	}
	
	Operand field_offset = new_Operand();
	field_offset->kind = CONSTANT;
	field_offset->u.value = count;

	Operand array_start = new_Temp();
	InterCode add_code = new_InterCode();
	add_code->kind = ADD;
	add_code->u.binop.result = array_start;
	add_code->u.binop.op1 = start;
	add_code->u.binop.op2 = field_offset;
	InterCodeNode node1 = produceNodeByCode(add_code);

	Operand t1 = new_Temp();
	InterCodeNode node2 = translate_Exp(index_exp,t1);
	
	Operand t2 = new_Temp();
	Operand length4 = new_Operand();
	length4->kind = CONSTANT;
	length4->u.value = 4;

	InterCode mul_code = new_InterCode();
	mul_code->kind = MUL;
	mul_code->u.binop.result = t2;
	mul_code->u.binop.op1 = t1;
	mul_code->u.binop.op2 = length4;
	InterCodeNode node3 = produceNodeByCode(mul_code);

	Operand addr = new_Temp();
	InterCode add_code1 = new_InterCode();
	add_code1->kind = ADD;
	add_code1->u.binop.result = addr;
	add_code1->u.binop.op1 = array_start;
	if(t1->kind == CONSTANT){
		t1->u.value = t1->u.value * 4;
		add_code1->u.binop.op2 = t1;
		node3 = NULL;
	}else{
		add_code1->u.binop.op2 = t2;
	}

	InterCodeNode node4 = produceNodeByCode(add_code1);

	// Operand new_addr = new_Operand();
	// new_addr->kind = ADDRESS;
	// new_addr->u.temp_no = addr->u.temp_no;
	// Operand data = new_Temp();
	// InterCode assign_code = new_InterCode();
	// assign_code->kind = ASSIGN;
	// assign_code->u.assign.left = data;
	// assign_code->u.assign.right = new_addr;
	// InterCodeNode node5 = produceNodeByCode(assign_code);
	
	//show(node4);
	place->kind = ADDRESS;
	place->u.temp_no = addr->u.temp_no;
	
	InterCodeNode temp12 = produceNodeByNode(node1,node2);
	InterCodeNode temp123 = produceNodeByNode(temp12,node3);
	InterCodeNode temp1234 = produceNodeByNode(temp123,node4);
	return temp1234;
	//return produceNodeByNode(temp1234,node5);
}
