#ifndef __ENV_H__
#define __ENV_H__

#include "node.h"

struct Var{
	char* key;
	struct Node* node;
	//関数が呼ばれたら++call, 抜けると--calll
	//ネストが作られたら++nest, 抜けると--nest
	uint32_t call, nest;
};

void env_init();
struct Node* find(char* key);
uint32_t resist(char* key, struct Node* node);
uint32_t current_fptr();
void into_scope();
void exit_scope();
bool into_func(uint32_t pos);
void exit_func();
void env_dump();

int env_var_size();
struct Var* env_vars();
#endif
