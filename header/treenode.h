#ifndef _NODE_H_
#define _NODE_H_

extern int g_error;					//判断是否有错误产生的全局变量
extern struct Treenode *g_treeroot;	//指向语法树根结点的全局变量

struct YYLTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
};
struct YYLTYPE yylloc;

typedef enum{
	NODE_INT,
	NODE_FLOAT,
	NODE_ID,
	NODE_TERMINUS,
	NODE_NOTERMINUS
}node_type;

typedef union node_value{
	int ival;
	float fval;
	char idval[30];
	char operval[30];
}node_value;

typedef struct Treenode{
	node_type type;
	int lineno;
	int ival;
	float fval;
	char idval[30];
	char operval[30];
	char name[30];

	int child_num;
	struct Treenode **node;
} treenode;

#endif
