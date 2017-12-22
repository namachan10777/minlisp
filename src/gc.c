#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc.h"
#include "macro.h"
#include "env.h"
//ノードの数
uint64_t node_num = 0;
//現在確保されてるノード格納スペースの大きさ
uint64_t reserved_size = 1024;
//ノード格納スペースへのポインタ
struct Node **heap;

void gc_init() {
	heap = (struct Node**)malloc(sizeof(struct Node*) * reserved_size);
}

struct Node* gc_alloc() {
	if (node_num + 1 > reserved_size)
		RESERVE(struct Node*, heap, reserved_size);
	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
	heap[node_num++] = node;
	return node;
}

//internal function
void mark_rec(struct Node* node) {
	node->visited = true;
	if (node->tag == Sexp) {
		mark_rec(node->sexp.car);
		mark_rec(node->sexp.cdr);
	}
}

void mark() {
	struct Var* var_table = env_vars();
	size_t var_size = env_var_size();
	for (size_t i = 0; i < var_size; ++i) {
		mark_rec(var_table[i].node);
	}
}

void sweep() {
	for (size_t i = 0; i < node_num; ++i) {
		if (heap[i] == NULL) continue;
		if (heap[i]->visited) {
			heap[i]->visited = false;
		}
		else {
			free(heap[i]);
			heap[i] = NULL;
		}
	}
}

void compaction() {
	size_t last_idx = 0;
	for (size_t i = 0; i < node_num; ++i) {
		if (heap[i] == NULL){
			--node_num;
			continue;
		}
		else {
			heap[last_idx] = heap[i];
			++last_idx;
		}
	}
}

void gc_collect() {
	mark();
	sweep();
	compaction();
}
