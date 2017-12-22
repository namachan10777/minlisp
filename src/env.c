#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "node.h"
#include "macro.h"

struct Var* vars;
uint32_t var_reserved_size = 256;
uint32_t var_size = 0;

uint32_t call_level = 0, nest_level = 0;

size_t* callstack;
uint32_t callstack_reserved_size = 256;
uint32_t callstack_size = 0;
// internal functions
int64_t find_idx (char* key) {
	for (int64_t i = var_size - 1; i >= 0 && vars[i].call == call_level; --i) {
		if (strcmp(key, vars[i].key) == 0)
			return i;
	}
	if (callstack_size > 0) {
		for (int64_t i = callstack[callstack_size-1]; i >= 0; --i) {
			if (strcmp(key, vars[i].key) == 0)
				return i;
		}
	}
	return -1;
}

// exernal functions
void env_init() {
	vars = malloc(sizeof(struct Node*) * var_reserved_size);
	callstack = malloc(sizeof(uint32_t*) * callstack_size);
}

void into_scope() {
	++nest_level;
}

void exit_scope() {
	for (int64_t i = var_size - 1; i >= 0; --i) {
		if (vars[i].nest >= nest_level && vars[i].call == call_level)
			--var_size;
		else
			break;
	}
	--nest_level;
}

bool into_func(char* key) {
	int64_t fptr;
	if ((fptr = find_idx(key)) < 0) 
		return false;
	if (callstack_size >= callstack_reserved_size)
		RESERVE(uint32_t, callstack, callstack_reserved_size);
	callstack[callstack_size++] = fptr;
	++call_level;
	return true;
}

void exit_func() {
	for(int64_t i = var_size; i >= 0; --i) {
		if (vars[i].call >= call_level) {
			//変数名はGCの管理対象外なので参照しなくなった時点で消す
			free(vars[i].key);
			--var_size;
		}
	}
	--callstack_size;
	--call_level;
}

struct Node* find(char* key) {
	int64_t idx;
	if ((idx = find_idx(key)) < 0)
		return NULL;
	return vars[idx].node;
}

void resist(char* key, struct Node* node) {
	if (var_size >= var_reserved_size)
		RESERVE(struct Var, vars, callstack_reserved_size);
	struct Var var = {key, node, call_level, nest_level};
	vars[var_size++] = var;
}

void env_dump() {
	printf("------------ env dump -------------\n");
	printf("call %d nest %d\n", call_level, nest_level);
	printf("----------- vars dump -------------\n");
	printf("                name | call | nest\n");
	for (size_t i = 0; i < var_size; ++i) {
		printf("%20s | %4d | %3d\n", vars[i].key, vars[i].call, vars[i].nest);
	}
	printf("----------- stack dump -------------\n");
	if (callstack_size > 0) {
		for (size_t i = 0; i < callstack_size - 1; ++i) {
			printf("%ld ->", callstack[i]);
		}
		printf("%ld\n", callstack[callstack_size]);
	}
}

int env_var_size() {
	return var_size;
}

struct Var* env_vars() {
	return vars;
}
