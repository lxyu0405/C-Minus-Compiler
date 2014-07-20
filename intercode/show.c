#include <stdio.h>
#include <stdlib.h>
#include "../header/intercode.h"


/*
 * show.c
 */

void optimize(InterCodeNode head);
void show(InterCodeNode head);
void analyzeCode(InterCode code);
void analyzeOperand(Operand op);

void writeFile(InterCodeNode head,FILE* file);
void writeCode(InterCode code,FILE* file);
void writeOperand(Operand op,FILE* file);

void optimize(InterCodeNode head){
	InterCodeNode p = head;
	while(p->next != NULL){
		InterCodeNode q = p->next;
		InterCode calcu = p->code;
		InterCode assig = q->code;
		if(calcu->kind == ADD || calcu->kind == SUB
				|| calcu->kind == MUL || calcu->kind == DIV){
			Operand result = calcu->u.binop.result;
			if(assig->kind == ASSIGN){
				Operand right = assig->u.assign.right;
				Operand left = assig->u.assign.left;
				if(result == right && (left->kind == VARIABLE || left->kind == TEMP)){
					calcu->u.binop.result = assig->u.assign.left;
					p->next = q->next;
					(q->next)->prev = p;
				}
			}
		}
		p = p->next;
	}
	return ;
}

void show(InterCodeNode head){
	InterCodeNode p = head;
	while(p != NULL){
		analyzeCode(p->code);
		p = p->next;
	}
}

void analyzeCode(InterCode code){
	switch(code->kind){
        case ASSIGN:
            analyzeOperand(code->u.assign.left);
            printf(" := ");
            analyzeOperand(code->u.assign.right);
            break;
        case ADD:
            analyzeOperand(code->u.binop.result);
            printf(" := ");
            analyzeOperand(code->u.binop.op1);
            printf(" + ");
            analyzeOperand(code->u.binop.op2);
            break;
        case DIV:        
            analyzeOperand(code->u.binop.result);
            printf(" := ");
            analyzeOperand(code->u.binop.op1);
            printf(" / ");
            analyzeOperand(code->u.binop.op2);
            break;
		case SUB:        
            analyzeOperand(code->u.binop.result );
            printf(" := ");
            analyzeOperand(code->u.binop.op1 );
            printf(" - ");
            analyzeOperand(code->u.binop.op2 );
            break;
        case MUL:        
            analyzeOperand(code->u.binop.result );
            printf(" := ");
            analyzeOperand(code->u.binop.op1 );
            printf(" * ");
            analyzeOperand(code->u.binop.op2 );
            break;
        case RELOP:
            printf("IF ");
            analyzeOperand(code->u.rel.op1 );
            printf(" ");
            analyzeOperand(code->u.rel.relop_op );
            printf(" ");
            analyzeOperand(code->u.rel.op2 );
            printf(" GOTO ");
            analyzeOperand(code->u.rel.label_op );
            break;
        case GOTO:        
            printf("GOTO ");
            analyzeOperand(code->u.gto.label_op );
            break;
		case LABEL:        
            printf("LABEL ");
            analyzeOperand(code->u.label.label_op );
            printf(" :");
            break;
        case READ:
            printf("READ ");
            analyzeOperand(code->u.read.arg);
            break;
        case WRITE:
            printf("WRITE ");
            analyzeOperand(code->u.write.arg );
            break;
		case CALL:
            analyzeOperand(code->u.call.result );
            printf(" := CALL ");
            analyzeOperand(code->u.call.function );
            break;
        case RETURN:
            printf("RETURN ");
            analyzeOperand(code->u.retn.return_op );
            break;
        case FUNCTION:
            printf("FUNCTION ");
            analyzeOperand(code->u.function.func );
            printf(" :");
            break;
        case PARAM:
            printf("PARAM ");
            analyzeOperand(code->u.param.var );
            break;
        case DEC:
            printf("DEC ");
            analyzeOperand(code->u.dec.var );
            printf(" ");
            analyzeOperand(code->u.dec.size );
            break;
		case ARG:
            printf("ARG ");
            analyzeOperand(code->u.arg.arg_op );
            break;
    }
    printf("\n");
}

void analyzeOperand(Operand op){
    switch(op->kind){
        case VARIABLE:
            printf("v%d",op->u.var_no);
            break;
        case CONSTANT:
            printf("#%d",op->u.value);
            break;
        case ADDRESS:
			if(op->u.var_no == 0)
            	printf("*t%d",op->u.temp_no);
			else
            	printf("*v%d",op->u.var_no);
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


void writeFile(InterCodeNode head,FILE* file){
	InterCodeNode p = head;
	while(p != NULL){
		writeCode(p->code,file);
		p = p->next;
	}
}

void writeCode(InterCode code,FILE* file){
	switch(code->kind){
        case ASSIGN:
            writeOperand(code->u.assign.left,file);
            printf(" := ");
            fprintf(file," := ");
            writeOperand(code->u.assign.right,file);
            break;
        case ADD:
            writeOperand(code->u.binop.result,file);
            printf(" := ");
            fprintf(file," := ");
            writeOperand(code->u.binop.op1,file);
            printf(" + ");
            fprintf(file," + ");
            writeOperand(code->u.binop.op2,file);
            break;
        case DIV:        
            writeOperand(code->u.binop.result,file);
            printf(" := ");
            fprintf(file," := ");
            writeOperand(code->u.binop.op1,file);
            printf(" / ");
            fprintf(file," / ");
            writeOperand(code->u.binop.op2,file);
            break;
		case SUB:        
            writeOperand(code->u.binop.result ,file);
            printf(" := ");
            fprintf(file," := ");
            writeOperand(code->u.binop.op1 ,file);
            printf(" - ");
            fprintf(file," - ");
            writeOperand(code->u.binop.op2 ,file);
            break;
        case MUL:        
            writeOperand(code->u.binop.result ,file);
            printf(" := ");
            fprintf(file," := ");
            writeOperand(code->u.binop.op1 ,file);
            printf(" * ");
            fprintf(file," * ");
            writeOperand(code->u.binop.op2 ,file);
            break;
        case RELOP:
            printf("IF ");
            fprintf(file,"IF ");
            writeOperand(code->u.rel.op1 ,file);
            printf(" ");
            fprintf(file," ");
            writeOperand(code->u.rel.relop_op ,file);
            printf(" ");
            fprintf(file," ");
            writeOperand(code->u.rel.op2 ,file);
            printf(" GOTO ");
            fprintf(file," GOTO ");
            writeOperand(code->u.rel.label_op ,file);
            break;
        case GOTO:        
            printf("GOTO ");
            fprintf(file,"GOTO ");
            writeOperand(code->u.gto.label_op ,file);
            break;
		case LABEL:        
            printf("LABEL ");
            fprintf(file,"LABEL ");
            writeOperand(code->u.label.label_op ,file);
            printf(" :");
            fprintf(file," :");
            break;
        case READ:
            printf("READ ");
            fprintf(file,"READ ");
            writeOperand(code->u.read.arg,file);
            break;
        case WRITE:
            printf("WRITE ");
            fprintf(file,"WRITE ");
            writeOperand(code->u.write.arg ,file);
            break;
		case CALL:
            writeOperand(code->u.call.result ,file);
            printf(" := CALL ");
            fprintf(file," := CALL ");
            writeOperand(code->u.call.function ,file);
            break;
        case RETURN:
            printf("RETURN ");
            fprintf(file,"RETURN ");
            writeOperand(code->u.retn.return_op ,file);
            break;
        case FUNCTION:
            printf("FUNCTION ");
            fprintf(file,"FUNCTION ");
            writeOperand(code->u.function.func ,file);
            printf(" :");
            fprintf(file," :");
            break;
        case PARAM:
            printf("PARAM ");
            fprintf(file,"PARAM ");
            writeOperand(code->u.param.var ,file);
            break;
        case DEC:
            printf("DEC ");
            fprintf(file,"DEC ");
            writeOperand(code->u.dec.var ,file);
            printf(" ");
            fprintf(file," ");
            writeOperand(code->u.dec.size ,file);
            break;
		case ARG:
            printf("ARG ");
            fprintf(file,"ARG ");
            writeOperand(code->u.arg.arg_op ,file);
            break;
    }
    printf("\n");
    fprintf(file,"\n");
}

void writeOperand(Operand op,FILE* file){
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

