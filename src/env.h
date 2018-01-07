#ifndef __ENV_H__
#define __ENV_H__

#include "node.h"

enum BoxType {
	BlockHead,
	ScopeHead,
	Var,
	RVal
};

struct Box{
	enum BoxType tag;
	char* key;
	struct Node* node;
	//関数が呼ばれたら++call, 抜けると--calll
	//ネストが作られたら++nest, 抜けると--nest
};

void env_init();
void env_quit();
struct Node* find(char* key);
int resist(char* key, struct Node* node);
void stack(struct Node* node);
void into_scope();
void exit_scope();
bool into_func(int pos);
void exit_func();

int get_env_size();
struct Box* get_env();
#endif
