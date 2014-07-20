#include <stdio.h>
#include <stdlib.h>
#include "../header/mipscode.h"
#include "../header/intercode.h"

struct FunctionField* calStackField(InterCodeNode field){
	struct FunctionField *func_list = NULL;
	//check every operation in the function
	InterCodeNode p = field;
	struct FunctionField *new_func = NULL;
	while(p != NULL){
		if(p->code->kind == FUNCTION){
			if(new_func != NULL){
				addNewFuncField(&func_list,new_func);
			}
			new_func = (struct FunctionField*)malloc(sizeof(struct FunctionField));
			new_func->func_name = p->code->u.function.func->u.func_name;
			new_func->arg_num = 0;
			new_func->param_num = 0;
			new_func->param_use = 0;
			new_func->var_fp = new_func->var_sp = NULL;
		}
		if(new_func != NULL){
			analyzeVarType(p->code,new_func);
		}
		p = p->next;
	}
	if(new_func != NULL){
		addNewFuncField(&func_list,new_func);
	}
	return func_list;
}

void addNewFuncField(struct FunctionField **list,struct FunctionField *func){
	if(func == NULL){
		//deprintf("add function is NULL\n");
		return ;
	}else if(*list == NULL){
		//deprintf("list head is NULL\n");
		*list = func;
		return ;
	}else{
		struct FunctionField *p = *list;
		while(p->next != NULL){
			p = p->next;
		}
		func->next = NULL;
		p->next = func;
	}
}

void analyzeVarType(InterCode code,struct FunctionField *func){
	//analyze variables' types
	switch(code->kind){
		case ASSIGN:
		    addNewVarToField(code->u.assign.left,INT_,func);
		    addNewVarToField(code->u.assign.right,INT_,func);
		    break;
		case ADD:
		    addNewVarToField(code->u.binop.result,INT_,func);
		    addNewVarToField(code->u.binop.op1,INT_,func);
		    addNewVarToField(code->u.binop.op2,INT_,func);
		    break;
		case DIV:        
		    addNewVarToField(code->u.binop.result,INT_,func);
		    addNewVarToField(code->u.binop.op1,INT_,func);
		    addNewVarToField(code->u.binop.op2,INT_,func);
		    break;
		case SUB:        
			addNewVarToField(code->u.binop.result,INT_,func);
		    addNewVarToField(code->u.binop.op1,INT_,func);
		    addNewVarToField(code->u.binop.op2,INT_,func);
		    break;
		case MUL:        
			addNewVarToField(code->u.binop.result,INT_,func);
		    addNewVarToField(code->u.binop.op1,INT_,func);
		    addNewVarToField(code->u.binop.op2,INT_,func);
		    break;
		case RELOP:
		    addNewVarToField(code->u.rel.op1,INT_,func);
		    addNewVarToField(code->u.rel.op2,INT_,func);
		    break;
		case READ:
		    addNewVarToField(code->u.read.arg,INT_,func);
		    break;
		case WRITE:
		    addNewVarToField(code->u.write.arg,INT_,func);
		    break;
		case CALL:
		    addNewVarToField(code->u.call.result,INT_,func);
		    break;
		case RETURN:
		    addNewVarToField(code->u.retn.return_op,INT_,func);
		    break;
		case PARAM:
		    addNewVarToField(code->u.param.var,INT_,func);
			func->param_num++;
		    break;
		case DEC:
		    addNewVarToField(code->u.dec.var,code->u.dec.size->u.value,func);
		    break;
		case ARG:
		    addNewVarToField(code->u.arg.arg_op,INT_,func);
			Operand op = (Operand)malloc(sizeof(struct Operand_));
			op->kind = ARGOP;
			op->u.arg_no = func->arg_num;
		    addNewVarToField(op,INT_,func);
			func->arg_num++;
		    break;
    	}
}

void addNewVarToField(Operand op,int size,struct FunctionField *func){
	//add new variable to the function field
	struct VarInStack *new_var = (struct VarInStack*)malloc(sizeof(struct VarInStack));
	new_var->operand = op;
	new_var->next = NULL;
	//查找该函数活动域中是否有该操作数
	struct VarInStack *temp = func->var_fp;
	while(temp != NULL){
		if(equalOperand(temp->operand,op) == 1)
			break;
		temp = temp->next;
	}
	//新的操作数
	if(temp == NULL){
		if(func->var_fp == NULL){
			//empty
			new_var->offset = size;
			func->var_fp = new_var;
			func->var_sp = new_var;
		}else{
			new_var->offset = func->var_sp->offset + size;
			func->var_sp->next = new_var;
			func->var_sp = new_var;
		}
	}
}

int equalOperand(Operand op1,Operand op2){
	//check whether two operands are equal
	if(op1->kind == ADDRESS){
		if(op2->kind == TEMP && op1->u.temp_no != 0 && op2->u.temp_no == op1->u.temp_no)
			return 1;
		if(op2->kind == ADDRESS){
			if(op1->u.temp_no != 0 && op1->u.temp_no == op2->u.temp_no)
				return 1;
			if(op1->u.var_no != 0 && op1->u.var_no == op2->u.var_no)
				return 1;
		}
		
	}else if(op1->kind == TEMP){
		if(op2->kind == TEMP && op1->u.temp_no == op2->u.temp_no)
			return 1;
		if(op2->kind == ADDRESS && op2->u.temp_no != 0 && op1->u.temp_no == op2->u.temp_no)
			return 1;
	}else if(op1->kind == VARIABLE || op1->kind == REFERENCE){
		if(op2->kind == VARIABLE && op1->u.var_no == op2->u.var_no)
			return 1;
		if(op2->kind == REFERENCE && op1->u.var_no == op2->u.var_no)
			return 1;
	}else if(op1->kind == CONSTANT && op2->kind == CONSTANT){
		if(op1->u.value == op2->u.value)
			return 1;
	}else if(op1->kind == ARGOP && op2->kind == ARGOP){
		if(op1->u.arg_no == op2->u.arg_no)
			return 1;
	}
	return 0;
}
