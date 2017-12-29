#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc.h"
#include "macro.h"
#include "env.h"
//ノードの数
uint64_t node_num = 0;
//GCを呼ばれたけども実行しなかったもののカウント。実行するとリセットする。
uint64_t call_cnt = 0;
//32回呼ばれるごとにGCを実行
const uint64_t call_cnt_thresh = 32;
//現在確保されてるノード格納スペースの大きさ
uint64_t reserved_size = 1024;
//ノード格納スペースへのポインタ
struct Node **heap;

void gc_init() {
	INIT(struct Node*, heap, reserved_size);
}

void gc_quit() {
	free(heap);
}

struct Node* gc_alloc() {
	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
	APPEND(struct Node*, heap, reserved_size, node_num, node);
	return node;
}

void gc_free(struct Node* node) {
	if (node != NULL) {
		switch (node->tag) {
		case Nil:
		case Bool:
		case Num:
		case Pair:
		case BFun: 
		case SForm: {
				free(node);
				break;
			}
		case Str: {
				free(node->str);
				free(node);
				break;
			}
		case Symbol: {
				free(node->symbol);
				free(node);
				break;
			}
		case Fun: {
				for (size_t i = 0; i < node->fun.arg_num; ++i) {
					free(node->fun.args[i]);
				}
				free(node);
				break;
			}
		}
	}
}

//internal function
void mark_rec(struct Node* node) {
	node->visited = true;
	if (node->tag == Pair) {
		mark_rec(node->pair.car);
		mark_rec(node->pair.cdr);
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
			gc_free(heap[i]);
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
	if (call_cnt < call_cnt_thresh) {
		++call_cnt;
	}
	else {
		mark();
		sweep();
		compaction();
	}
}
