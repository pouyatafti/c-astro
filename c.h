#ifndef __CASTRO_C_H__
#define __CASTRO_C_H__

#define nil ( (void *) 0 )

typedef struct llnode llnode;
struct llnode {
	llnode *next;
	void *data;
};

typedef struct ll2node ll2node;
struct ll2node {
	ll2node *prev;
	ll2node *next;
	void *data;
};

#endif