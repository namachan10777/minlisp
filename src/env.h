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
void resist(char* key, struct Node* node);
void into_scope();
void exit_scope();
bool into_func(char* key);
void exit_func();
void env_dump();
#endif
