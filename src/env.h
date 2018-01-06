#ifndef __ENV_H__
#define __ENV_H__

#include "node.h"

struct Var{
	bool is_head;
	char* key;
	struct Node* node;
	//関数が呼ばれたら++call, 抜けると--calll
	//ネストが作られたら++nest, 抜けると--nest
	int call, nest;
};

void env_init();
void env_quit();
struct Node* find(char* key);
int resist(char* key, struct Node* node);
int resist_real_arg(char* key, struct Node* node);
int current_fptr();
void into_scope();
void exit_scope();
void start_resist_real_arg();
bool enter_func(int pos);
void exit_func();
void env_dump();

int env_var_size();
struct Var* env_vars();
#endif
