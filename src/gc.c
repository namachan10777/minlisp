#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "gc.h"
#include "macro.h"
#include "env.h"
//ノードの数
int node_num = 0;
//GCを呼ばれたけども実行しなかったもののカウント。実行するとリセットする。
int call_cnt = 0;
//32回呼ばれるごとにGCを実行
const int call_cnt_thresh = 32;
//現在確保されてるノード格納スペースの大きさ
int reserved_size = 1024;
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
	node->visited = false;
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
				for (int i = 0; i < node->fun.arg_num; ++i) {
					free(node->fun.args[i]);
				}
				free(node->fun.args);
				free(node);
				break;
			}
		}
	}
}

//internal function
void mark_rec(struct Node* node) {
	node->visited = true;
	switch (node->tag) {
	case Nil:
	case Bool:
	case Num:
	case Str:
	case Symbol:
	case BFun: 
	case SForm:
		break;
	case Pair:
		mark_rec(node->pair.car);
		mark_rec(node->pair.cdr);
		break;
	case Fun:
		mark_rec(node->fun.body);
	}
}

void mark() {
	struct Box* env = get_env();
	int env_size = get_env_size();
	for (int i = 0; i < env_size; ++i) {
		mark_rec(env[i].node);
	}
}

void sweep() {
	for (int i = 0; i < node_num; ++i) {
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
	int last_idx = 0;
	for (int i = 0; i < node_num; ++i) {
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
