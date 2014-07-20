#ifndef _MIPSCODE_H_
#define _MIPSCODE_H_

#define INT_ sizeof(int)

struct VarInStack{
    Operand operand;
    int offset;
    struct VarInStack *next;
};

struct FunctionField{
	char *func_name;
	int arg_num;
	int param_num;
	int param_use;
	struct VarInStack *var_fp,*var_sp;		
	struct FunctionField *next;
};

void showOperand();
void presetMIPS(FILE* f);
int getOffset(Operand op);
void printFunctionFields();
void writeOperandMIPS(Operand op,FILE* file);
void writeOperandMIPS(Operand op,FILE* file);
void generateMIPS(InterCodeNode head,FILE* file);
void interCodeToMIPS(InterCode code,FILE* file);
void dataImmToRdest(Operand op,FILE* file,int index);
void dataRdestToImm(Operand op,FILE* file,int index);

struct FunctionField* getCurrentFunction(char *func_name);
void analyzeVarType(InterCode code,struct FunctionField *func);
struct FunctionField* calStackField(InterCodeNode field);
void addNewVarToField(Operand op,int size,struct FunctionField *func);
int equalOperand(Operand op1,Operand op2);
void addNewFuncField(struct FunctionField **list,struct FunctionField *func);

#endif
