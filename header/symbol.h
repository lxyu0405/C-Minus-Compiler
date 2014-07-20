#ifndef _TABLE_H_
#define _TABLE_H_

#include "treenode.h"

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct ExternList_* ExternList;

typedef unsigned int uint;
typedef unsigned short uint16;
typedef unsigned long uint32;


struct Type_
{
	enum { basic, array, structure } kind;
	union
	{
		//基本类型 0-int 1-float
		int basic;
		//数组类型 信息包括元素类型与数组大小
		struct{ Type elem; int size; } array;
		//结构体类型 信息是一个链表
		FieldList structure;
	}u;
};

struct FieldList_
{
	char *name;			// 域的名字
	Type type;			// 域的类型
	FieldList tail;		// 下一个域
};

struct ExternList_
{
	int line;
	int flag;
	FieldList func;
};


		//插入函数声明





#endif
