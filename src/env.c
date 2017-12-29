#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "node.h"
#include "macro.h"
#include "util.h"
#include "gc.h"

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
	INIT(struct Var, vars, var_reserved_size);
	INIT(size_t, callstack, callstack_reserved_size);

	resist("+", alloc_bfun(Add));
	resist("-", alloc_bfun(Sub));
	resist("*", alloc_bfun(Mul));
	resist("/", alloc_bfun(Div));
	resist("%", alloc_bfun(Mod));
	resist("not", alloc_bfun(Not));
	resist("and", alloc_bfun(And));
	resist("or", alloc_bfun(Or));
	resist("cons", alloc_bfun(Cons));
	resist("cdr", alloc_bfun(Cdr));
	resist("car", alloc_bfun(Car));
	resist("cons", alloc_bfun(Cons));
	resist("list", alloc_bfun(List));

	resist("if", alloc_sform(If));
	resist("let", alloc_sform(Let));
	resist("quote", alloc_sform(Quote));
	resist("defun", alloc_sform(Defun));
	resist("lambda", alloc_sform(Lambda));
}

void env_quit() {
	for (size_t i = 0; i < var_size; ++i) {
		free(vars[i].key);
		gc_free(vars[i].node);
	}
	free(callstack);
	free(vars);
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

bool into_func(uint32_t pos) {
	APPEND(size_t, callstack, callstack_reserved_size, callstack_size, pos);
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

uint32_t resist(char* key, struct Node* node) {
	struct Var var = {deep_copy(key), node, call_level, nest_level};
	APPEND(struct Var, vars, var_reserved_size, var_size, var);
	return var_size - 1;
}

uint32_t current_fptr() {
	return var_size;
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
