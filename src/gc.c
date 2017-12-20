#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "gc.h"
#include "macro.h"
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
