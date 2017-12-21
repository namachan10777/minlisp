#ifndef __ENV_H__
#define __ENV_H__

#include "node.h"
void env_init();
struct Node* find(char* key);
void resist(char* key, struct Node* node);
void into_scope();
void exit_scope();
bool into_func(char* key);
void exit_func();
void env_dump();
#endif
