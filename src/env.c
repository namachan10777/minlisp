#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "node.h"
#include "util.h"
#include "gc.h"

struct Box* env;
int env_reserved_size = 256;
int env_size = 0;

int* callstack;
int callstack_reserved_size = 256;
int callstack_size = 0;

// internal functions
// 変数の参照の実装
// クロージャなので定義元の変数も参照できる。
//
struct Node* find (char* key) {
	//Block内から探す
	for (int i = env_size - 1; env[i].tag != BlockHead; --i) {
		if (env[i].tag == Var && strcmp(key, env[i].key) == 0)
			return env[i].node;
	}
	//funに登録されている定義場所情報から定義元変数を参照する
	if (callstack_size > 0) {
		for (int i = callstack[callstack_size-1]; i >= 0; --i) {
			if (env[i].tag == Var && strcmp(key, env[i].key) == 0)
				return env[i].node;
		}
	}
	return NULL;
}

// exernal functions
void env_init() {
	INIT(struct Box, env, env_reserved_size);
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
	for (int i = 0; i < env_size; ++i) {
		if (env[i].tag == Var)
			free(env[i].key);
	}
	free(callstack);
	free(env);
}

void into_scope() {
	struct Box part = {ScopeHead, NULL, NULL};
	APPEND(struct Box, env, env_reserved_size, env_size, part);
}

void exit_scope(struct Node* ret) {
	for (int i = env_size - 1; i >= 0 && (env[i].tag != BlockHead || env[i].tag != ScopeHead); --i) {
		//変数はGCの管理対象外なので参照しなくなった時点で消す
		free(env[i].key);
		--env_size;
		break;
	}
	if (env[env_size].tag == ScopeHead)
		--env_size;
	stack(ret);
}

bool into_func(int pos) {
	struct Box part = {BlockHead, NULL, NULL};
	APPEND(struct Box, env, env_reserved_size, env_size, part);
	APPEND(int, callstack, callstack_reserved_size, callstack_size, pos);
	return true;
}

void exit_func(struct Node* ret) {
	for(int i = env_size - 1; i >= 0 && env[i].tag != BlockHead; --i) {
		//変数名はGCの管理対象外なので参照しなくなった時点で消す
		free(env[i].key);
		--env_size;
	}
	//BlockHeadを消す
	--env_size;
	--callstack_size;
	stack(ret);
}

int resist(char* key, struct Node* node) {
	struct Box var = {Var, deep_copy(key), node};
	APPEND(struct Box, env, env_reserved_size, env_size, var);
	return env_size - 1;
}

void stack(struct Node* node) {
	struct Box rval = {RVal, NULL, node};
	APPEND(struct Box, env, env_reserved_size, env_size, rval);
}

int get_env_size() {
	return env_size;
}

struct Box* get_env() {
	return env;
}
