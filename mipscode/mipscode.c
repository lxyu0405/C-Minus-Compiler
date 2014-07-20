#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../header/mipscode.h"
#include "functionField.c"

struct FunctionField *fields;
struct FunctionField *g_current_func;

void generateMIPS(InterCodeNode head,FILE *file){
	//analyze function field
	fields = calStackField(head);
	//printFunctionFields();
	//initialize MIPS code
	presetMIPS(file);
	InterCodeNode p = head;
	while(p != NULL){
		interCodeToMIPS(p->code,file);
		p = p->next;
	}
}

struct FunctionField* getCurrentFunction(char *func_name){
	if(func_name == NULL){
		//deprintf("search parameter 'func_name' is NULL\n");
		return NULL;
	}
	struct FunctionField *p = fields;
	while(p != NULL){
		if(strcmp(func_name,p->func_name) == 0)
			return p;
		p = p->next;
	}
	return NULL;
} 


void interCodeToMIPS(InterCode code,FILE* file){
	char *relop_kind;
	switch(code->kind){
        case ASSIGN:
            dataImmToRdest(code->u.assign.right,file,2);
            printf("\t move $t1, $t2\n");
            fprintf(file,"\t move $t1, $t2\n");
            dataRdestToImm(code->u.assign.left,file,1);
            break;
        case ADD:
            dataImmToRdest(code->u.binop.op1,file,2);
            dataImmToRdest(code->u.binop.op2,file,3);
			printf("\t add $t1, $t2, $t3\n");
			fprintf(file,"\t add $t1, $t2, $t3\n");
           	dataRdestToImm(code->u.binop.result,file,1);
            break;
        case DIV:        
            dataImmToRdest(code->u.binop.op1,file,2);
            dataImmToRdest(code->u.binop.op2,file,3);
			printf("\t div $t1, $t2, $t3\n");
			fprintf(file,"\t div $t1, $t2, $t3\n");
			printf("\t mflo $t1\n");
			fprintf(file,"\t mflo $t1\n");
           	dataRdestToImm(code->u.binop.result,file,1);
            break;
        case SUB:        
            dataImmToRdest(code->u.binop.op1,file,2);
            dataImmToRdest(code->u.binop.op2,file,3);
			printf("\t sub $t1, $t2, $t3\n");
			fprintf(file,"\t sub $t1, $t2, $t3\n");
           	dataRdestToImm(code->u.binop.result,file,1);
            break;
        case MUL:        
            dataImmToRdest(code->u.binop.op1,file,2);
            dataImmToRdest(code->u.binop.op2,file,3);
			printf("\t mul $t1, $t2, $t3\n");
			fprintf(file,"\t mul $t1, $t2, $t3\n");
           	dataRdestToImm(code->u.binop.result,file,1);
            break;
        case RELOP:
            dataImmToRdest(code->u.rel.op1,file,1);
            dataImmToRdest(code->u.rel.op2,file,3);
			if(strcmp(code->u.rel.relop_op->u.relop_name,"==") == 0)
				relop_kind = "beq";
			if(strcmp(code->u.rel.relop_op->u.relop_name,"!=") == 0)
				relop_kind = "bne";
			if(strcmp(code->u.rel.relop_op->u.relop_name,">=") == 0)
				relop_kind = "bge";
			if(strcmp(code->u.rel.relop_op->u.relop_name,"<=") == 0)
				relop_kind = "ble";
			if(strcmp(code->u.rel.relop_op->u.relop_name,">") == 0)
				relop_kind = "bgt";
			if(strcmp(code->u.rel.relop_op->u.relop_name,"<") == 0)
				relop_kind = "blt";
            printf("\t %s $t1, $t3, ",relop_kind);
            fprintf(file,"\t %s $t1, $t3, ",relop_kind);
            writeOperandMIPS(code->u.rel.label_op,file);
            printf("\n");
            fprintf(file,"\n");
            break;
        case GOTO:        
            printf("\t j ");
            fprintf(file,"\t j ");
            writeOperandMIPS(code->u.gto.label_op,file);
			printf("\n");
            fprintf(file,"\n");
            break;
        case LABEL:        
            writeOperandMIPS(code->u.label.label_op,file);
            printf(":\n");
            fprintf(file,":\n");
            break;
        case READ:
            printf("\t addi $sp, $sp, -4\n");
            fprintf(file,"\t addi $sp, $sp, -4\n");
            printf("\t sw $ra, 0($sp)\n");
            fprintf(file,"\t sw $ra, 0($sp)\n");
            printf("\t jal read\n");
            fprintf(file,"\t jal read\n");
            printf("\t lw $ra, 0($sp)\n");
            fprintf(file,"\t lw $ra, 0($sp)\n");
            printf("\t addi $sp, $sp, 4\n");
            fprintf(file,"\t addi $sp, $sp, 4\n");
            printf("\t move $t1, $v0\n");
            fprintf(file,"\t move $t1, $v0\n");
            dataRdestToImm(code->u.read.arg,file,1);
            break;
        case WRITE:
            dataImmToRdest(code->u.write.arg,file,1);
            printf("\t move $a0, $t1\n");
            fprintf(file,"\t move $a0, $t1\n");
            printf("\t addi $sp, $sp, -4\n");
            fprintf(file,"\t addi $sp, $sp, -4\n");
            printf("\t sw $ra, 0($sp)\n");
            fprintf(file,"\t sw $ra, 0($sp)\n");
            printf("\t jal write\n");
            fprintf(file,"\t jal write\n");
            printf("\t lw $ra, 0($sp)\n");
            fprintf(file,"\t lw $ra, 0($sp)\n");
            printf("\t addi $sp, $sp, 4\n");
            fprintf(file,"\t addi $sp, $sp, 4\n");
            printf("\t move $v0, $0\n");
            fprintf(file,"\t move $v1, $0\n");
            break;
        case CALL:
			printf("\t addi $sp, $sp, -4\n");
			fprintf(file,"\t addi $sp, $sp, -4\n");
			printf("\t sw $ra, 0($sp)\n");
			fprintf(file,"\t sw $ra, 0($sp)\n");
			printf("\t jal %s\n",code->u.call.function->u.func_name);
			fprintf(file,"\t jal %s\n",code->u.call.function->u.func_name);
			printf("\t lw $ra, 0($sp)\n");
			fprintf(file,"\t lw $ra, 0($sp)\n");
			printf("\t addi $sp, $sp, 4\n");
			fprintf(file,"\t addi $sp, $sp, 4\n");
			printf("\t move $t1, $v0\n");
			fprintf(file,"\t move $t1, $v0\n");
            dataRdestToImm(code->u.read.arg,file,1);
            break;
        case RETURN:
            dataImmToRdest(code->u.retn.return_op,file,1);
            printf("\t move $v0, $t1\n");
            fprintf(file,"\t move $v0, $t1\n");
            printf("\t addi $sp, $sp, %d\n",g_current_func->var_sp->offset);
            fprintf(file,"\t addi $sp, $sp, %d\n",g_current_func->var_sp->offset);
            printf("\t jr $ra\n");
            fprintf(file,"\t jr $ra\n");
            break;
        case FUNCTION:
			g_current_func = getCurrentFunction(code->u.function.func->u.func_name);
			if(g_current_func == NULL){
				//deprintf("'g_current_func' is NULL\n");
			}
            printf("\n%s:\n",code->u.function.func->u.func_name);
            fprintf(file,"\n%s:\n",code->u.function.func->u.func_name);
            printf("\t addi $sp, $sp, %d\n",-(g_current_func->var_sp->offset));
            fprintf(file,"\t addi $sp, $sp, %d\n",-(g_current_func->var_sp->offset));
            break;
        case PARAM:
			if(g_current_func->param_use >= 0 && g_current_func->param_use < 4){
				printf("\t move $t1, $a%d\n",g_current_func->param_use);
				fprintf(file,"\t move $t1, $a%d\n",g_current_func->param_use);
				g_current_func->param_use++;
				dataRdestToImm(code->u.param.var,file,1);
			}else if(g_current_func->param_use >= 4){
				int offset = g_current_func->var_sp->offset + (g_current_func->param_num - g_current_func->param_use - 4) * 4 + 28;
				printf("\t lw $t1, %d($sp)\n",offset);
				fprintf(file,"\t lw $t1, %d($sp)\n",offset);
				g_current_func->param_use++;
				dataRdestToImm(code->u.param.var,file,1);
			}
            break;
        case ARG:
			if(g_current_func->arg_num > 0 && g_current_func->arg_num <= 4){
				dataImmToRdest(code->u.arg.arg_op,file,1);
				g_current_func->arg_num--;
				printf("\t move $a%d, $t1\n",g_current_func->arg_num);
				fprintf(file,"\t move $a%d, $t1\n",g_current_func->arg_num);
			}else if(g_current_func->arg_num > 4){
				dataImmToRdest(code->u.arg.arg_op,file,1);
				g_current_func->arg_num--;
				Operand op = (Operand)malloc(sizeof(struct Operand_));
				op->kind = ARGOP;
				op->u.arg_no = g_current_func->arg_num;
				dataRdestToImm(op,file,1);
			}
            break;
    }
}

int getOffset(Operand op){
	struct VarInStack *p = g_current_func->var_fp;
	while(p != NULL){
		if(equalOperand(p->operand,op) == 1)
			break;
		p = p->next;
	}
	if(p == NULL){
		//deprintf("getOffset Error!\n");
		return 0;
	}
	return p->offset;
}

void dataImmToRdest(Operand op,FILE* file,int index){
	int offset = 0;
	switch(op->kind){
		case VARIABLE:
		    offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t lw $t%d, %d($sp)\n",index,offset);
			fprintf(file,"\t lw $t%d, %d($sp)\n",index,offset);
		    break;
		case CONSTANT:
			printf("\t li $t%d, %d\n",index,op->u.value);
			fprintf(file,"\t li $t%d, %d\n",index,op->u.value);
		    break;
		case ADDRESS:
			offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t lw $t5, %d($sp)\n",offset);
			fprintf(file,"\t lw $t5, %d($sp)\n",offset);
			printf("\t lw $t%d, 0($t5)\n",index);
			fprintf(file,"\t lw $t%d, 0($t5)\n",index);
		    break;
		case TEMP:
		    offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t lw $t%d, %d($sp)\n",index,offset);
			fprintf(file,"\t lw $t%d, %d($sp)\n",index,offset);
		    break;
		case REFERENCE:
			offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t addi $t%d, $sp, %d\n",index,offset);
			fprintf(file,"\t addi $t%d, $sp, %d\n",index,offset);
		    break;
    	}
}


void dataRdestToImm(Operand op,FILE* file,int index){
	int offset = 0;
	switch(op->kind){
		case VARIABLE:
		    offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t sw $t%d, %d($sp)\n",index,offset);
			fprintf(file,"\t sw $t%d, %d($sp)\n",index,offset);
		    break;
		case ADDRESS:
			offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t lw $t5, %d($sp)\n",offset);
			fprintf(file,"\t lw $t5, %d($sp)\n",offset);
			printf("\t sw $t%d, 0($t5)\n",index);
			fprintf(file,"\t sw $t%d, 0($t5)\n",index);
		    break;
		case TEMP:
			offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t sw $t%d, %d($sp)\n",index,offset);
			fprintf(file,"\t sw $t%d, %d($sp)\n",index,offset);	
			break;
		case ARGOP:
			offset = g_current_func->var_sp->offset - getOffset(op);
			printf("\t sw $t%d, %d($sp)\n",index,offset);
			fprintf(file,"\t sw $t%d, %d($sp)\n",index,offset);	
			break;
    	}
}

void presetMIPS(FILE *f){
	fputs(".data\n",f);
	fputs("_prompt: .asciiz \"Enter an integer:\"\n",f);
	fputs("_ret: .asciiz \"\\n\"\n",f);
	fputs(".globl main\n",f);
	fputs("\n.text\n",f);
	fputs("read:\n",f);
	fputs("\tli $v0 , 4\n",f);
	fputs("\tla $a0 , _prompt\n",f);
	fputs("\tsyscall\n",f);
	fputs("\tli $v0 , 5\n",f);
	fputs("\tsyscall\n",f);
	fputs("\tjr $ra\n",f);
	fputs("\nwrite:\n",f);
	fputs("\tli $v0 , 1\n",f);
	fputs("\tsyscall\n",f);
	fputs("\tli $v0 , 4\n",f);
	fputs("\tla $a0 , _ret\n",f);
	fputs("\tsyscall\n",f);
	fputs("\tmove $v0, $0\n",f);
	fputs("\tjr $ra\n",f);
}

void writeOperandMIPS(Operand op,FILE* file){
    switch(op->kind){
        case VARIABLE:
            printf("v%d",op->u.var_no);
            fprintf(file,"v%d",op->u.var_no);
            break;
        case CONSTANT:
            printf("#%d",op->u.value);
            fprintf(file,"#%d",op->u.value);
            break;
        case ADDRESS:
			if(op->u.var_no == 0){
            	printf("*t%d",op->u.temp_no);
            	fprintf(file,"*t%d",op->u.temp_no);
			}else{
            	printf("*v%d",op->u.var_no);
            	fprintf(file,"*v%d",op->u.var_no);
			}
            break;
        case TEMP:
            printf("t%d",op->u.temp_no);
            fprintf(file,"t%d",op->u.temp_no);
            break;
        case LABEL_OP:
            printf("label%d",op->u.label_no);
            fprintf(file,"label%d",op->u.label_no);
            break;
        case RELOP_OP:
            printf("%s",op->u.relop_name);
            fprintf(file,"%s",op->u.relop_name);
            break;
        case FUNC:
            printf("%s",op->u.func_name);
            fprintf(file,"%s",op->u.func_name);
            break;
        case SIZE:
            printf("%d",op->u.value);
            fprintf(file,"%d",op->u.value);
            break;
        case REFERENCE:
            printf("&v%d",op->u.var_no);
            fprintf(file,"&v%d",op->u.var_no);
            break;
    }	
}

void showOperand(Operand op){
    switch(op->kind){
        case VARIABLE:
            printf("v%d",op->u.var_no);
            break;
        case CONSTANT:
            printf("#%d",op->u.value);
            break;
        case ADDRESS:
			if(op->u.var_no == 0){
            	printf("*t%d",op->u.temp_no);
			}else{
            	printf("*v%d",op->u.var_no);
			}
            break;
        case TEMP:
            printf("t%d",op->u.temp_no);
            break;
        case LABEL_OP:
            printf("label%d",op->u.label_no);
            break;
        case RELOP_OP:
            printf("%s",op->u.relop_name);
            break;
        case FUNC:
            printf("%s",op->u.func_name);
            break;
        case SIZE:
            printf("%d",op->u.value);
            break;
        case REFERENCE:
            printf("&v%d",op->u.var_no);
            break;
    }	
}

void printFunctionFields(){
	struct FunctionField *body = fields;
	printf("\n******** Function Fields ********\n");
	while(body != NULL){
		printf("%s:\n",body->func_name);
		struct VarInStack *p = body->var_fp;
		while(p != NULL){
			printf("\t ");
			showOperand(p->operand);
			printf("\toffset:%d\n",p->offset);
			p = p->next;
		}
		body = body->next;
	}
	printf("\n******** Fields Display Over ********\n");
}
