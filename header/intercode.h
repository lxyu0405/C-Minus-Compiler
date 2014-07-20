#ifndef _INTERCODE_H_
#define _INTERCODE_H_


#include "treenode.h"
#include "symbol.h"

struct Var_info {
	char *name;
	int var_no;
	enum {BASIC, ARRAY, STRUCT} type;
	int size;
	struct Var_info *next; 
};

typedef struct Operand_* Operand;
struct Operand_ {
	enum { VARIABLE, CONSTANT, ADDRESS, TEMP, LABEL_OP, RELOP_OP, FUNC, SIZE, REFERENCE,ARGOP} kind;
	struct{
		int var_no;
		int value;
		int temp_no;
		int label_no;
		int arg_no;
		char *func_name;
		char relop_name[5];
	}u;
};

typedef struct OperandNode_* OperandNode;
struct OperandNode_ {
	Operand op;
	struct OperandNode_ *next;
};

typedef struct InterCode_* InterCode;
struct InterCode_ {
	enum { ASSIGN, ADD, SUB, MUL, RELOP, LABEL, DIV, GOTO, FUNCTION, PARAM, READ, WRITE, DEC, CALL, RETURN, ARG} kind;
	union{
		struct {Operand left, right;} assign;
		struct {Operand result, op1, op2;} binop;
		struct {Operand func;} function;
		struct {Operand var;} param;
		struct {Operand arg;} read;
		struct {Operand arg;} write;
		struct {Operand var, size;} dec;
		struct {Operand result, function;} call;
		struct {Operand return_op;} retn;
		struct {Operand label_op;} label;
		struct {Operand op1,relop_op,op2,label_op;} rel;
		struct {Operand label_op;} gto;
		struct {Operand arg_op;} arg;
	}u;

};

typedef struct InterCodeNode_* InterCodeNode;
struct InterCodeNode_ {
	struct InterCode_ *code;
	struct InterCodeNode_ *prev, *next;
};


#endif
