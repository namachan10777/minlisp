#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "node.h"
#include "macro.h"
#include "util.h"
#include "gc.h"

struct Var* vars;
int var_reserved_size = 256;
int var_size = 0;

int find_call_level = 0, resist_call_level = 0, nest_level = 0;

int* callstack;
int callstack_reserved_size = 256;
int callstack_size = 0;

bool is_head = true;
// internal functions
// 変数の参照の実装
// クロージャなので定義元の変数も参照できる。
//
int find_idx (char* key) {
	//call_levelが同じ間だけルックアップをする
	for (int i = var_size - 1; i >= 0 && vars[i].call == find_call_level; --i) {
		if (strcmp(key, vars[i].key) == 0)
			return i;
	}
	//funに登録されている定義場所情報から定義元変数を参照する
	if (callstack_size > 0) {
		for (int i = callstack[callstack_size-1]; i >= 0; --i) {
			if (strcmp(key, vars[i].key) == 0)
				return i;
		}
	}
	return -1;
}

// exernal functions
void env_init() {
	INIT(struct Var, vars, var_reserved_size);
	INIT(int, callstack, callstack_reserved_size);

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
	resist("<", alloc_bfun(Gret));
	resist(">", alloc_bfun(Less));
	resist("=", alloc_bfun(Eq));
	resist("print", alloc_bfun(Print));
	resist("progn", alloc_bfun(Progn));

	resist("if", alloc_sform(If));
	resist("let", alloc_sform(Let));
	resist("quote", alloc_sform(Quote));
	resist("defun", alloc_sform(Defun));
	resist("lambda", alloc_sform(Lambda));
}

void env_quit() {
	for (int i = 0; i < var_size; ++i) {
		free(vars[i].key);
		gc_free(vars[i].node);
	}
	free(callstack);
	free(vars);
}

void into_scope() {
	is_head = true;
	++nest_level;
}

void exit_scope() {
	for (int i = var_size - 1; i >= 0; --i) {
		//変数はGCの管理対象外なので参照しなくなった時点で消す
		free(vars[i].key);
		--var_size;
		if (vars[i].is_head)
			break;
	}
	--nest_level;
}

void start_resist_real_arg() {
	is_head = true;
	++resist_call_level;
}

int resist_real_arg(char* key, struct Node* node) {
	struct Var var = {is_head, deep_copy(key), node, resist_call_level, 0};
	APPEND(struct Var, vars, var_reserved_size, var_size, var);
	return var_size - 1;
}

bool enter_func(int pos) {
	find_call_level = resist_call_level;
	APPEND(int, callstack, callstack_reserved_size, callstack_size, pos);
	return true;
}

void exit_func() {
	for(int i = var_size; i >= 0; --i) {
		if (vars[i].call >= find_call_level) {
			//変数名はGCの管理対象外なので参照しなくなった時点で消す
			free(vars[i].key);
			--var_size;
			if (vars[i].is_head)
				break;
		}
	}
	--callstack_size;
	--resist_call_level;
	--find_call_level;
}

struct Node* find(char* key) {
	int idx;
	if ((idx = find_idx(key)) < 0)
		return NULL;
	return vars[idx].node;
}

int resist(char* key, struct Node* node) {
	struct Var var = {is_head, deep_copy(key), node, resist_call_level, nest_level};
	APPEND(struct Var, vars, var_reserved_size, var_size, var);
	is_head = false;
	return var_size - 1;
}


int current_fptr() {
	return var_size;
}

void env_dump() {
	printf("------------ env dump -------------\n");
	printf("rcall %d fcall %d nest %d\n", resist_call_level, find_call_level, nest_level);
	printf("----------- vars dump -------------\n");
	printf("                name | call | nest\n");
	for (int i = 0; i < var_size; ++i) {
		printf("%20s | %4d | %3d\n", vars[i].key, vars[i].call, vars[i].nest);
	}
	printf("----------- stack dump -------------\n");
	if (callstack_size > 0) {
		for (int i = 0; i < callstack_size - 1; ++i) {
			printf("%d ->", callstack[i]);
		}
		printf("%d\n", callstack[callstack_size]);
	}
}

int env_var_size() {
	return var_size;
}

struct Var* env_vars() {
	return vars;
}
