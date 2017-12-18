#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "env.h"
#include "node.h"

uint32_t scope_level = 0;
uint32_t reserved_level = 0;
struct Dict* env;

struct Pair {
	char* key;
	struct Node* node;
};

struct Dict {
	struct Pair* pairs;
	uint32_t size;
	uint32_t reserved_size;
};

void env_reserve (uint32_t new_size) {
	if (new_size > reserved_level) {
		struct Dict* new_env = malloc(sizeof(struct Dict*) * new_size);
		for (size_t i = 0; i < new_size; ++i) {
			if (i < scope_level)
				new_env[i] = env[i];
			else {
				struct Dict dic = { malloc(sizeof(struct Pair*) * 16), 0, 16 };
				new_env[i] = dic;
			}
		}
		env = new_env;
		reserved_level = new_size;
	}
}

struct Pair* reserve_pairs (struct Dict dict) {
	struct Pair* buf = malloc(sizeof(struct Pair*) * dict.reserved_size * 2);
	for (size_t i = 0; i < dict.size; ++i) {
		buf[i] = dict.pairs[i];
	}
	return buf;
}

void env_init() {
	env_reserve(16);
}

void into_scope() {
	if (scope_level >= reserved_level)
		env_reserve(reserved_level * 2);
	++scope_level;
}

void exit_scope() {
	if (scope_level >= reserved_level)
		env_reserve(reserved_level * 2);
	for (size_t i = 0; i < env[scope_level].size; ++i) {
		//NodeはGCが管理するが名前はGCの管理外なので参照しなくなると解放
		free(env[scope_level].pairs[i].key);
	}
	env[scope_level].size = 0;
	--scope_level;
}

void resist(char* key, struct Node* node) {
	if (env[scope_level].size >= env[scope_level].reserved_size)
		env[scope_level].pairs = reserve_pairs(env[scope_level]);
	struct Pair pair = {key, node};
	env[scope_level].pairs[env[scope_level].size] = pair;
	++env[scope_level].size;
}

struct Node* find(char* key) {
	for (int64_t level = scope_level; level >= 0; --level) {
		for (int64_t = env[level].size; i >= 0; ++i) {
			printf("%ld %ld\n", level, i);
			if (strcmp(key, env[level].pairs[i].key) == 0)
				return env[level].pairs[i].node;
		}
	}
	return NULL;
}
