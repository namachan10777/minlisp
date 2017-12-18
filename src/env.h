#ifndef __ENV_H__
#define __ENV_H__

#include "node.h"
void env_init();
struct Node* find(char* key);
void resist(char* key, struct Node* node);
void into_scope();
void exit_scope();
#endif
