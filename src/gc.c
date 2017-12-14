#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "gc.h"
//ノードの数
uint64_t node_num = 0;
//現在確保されてるノード格納スペースの大きさ
uint64_t reserved_size = 1024;
//ノード格納スペースへのポインタ
struct Node **heap;

void gc_init() {
	heap = (struct Node**)malloc(sizeof(struct Node*) * reserved_size);
}

//格納スペースを確保し直す。
void reserve() {
	struct Node **old_heap = heap;
	//確保し直すたびに倍の大きさの領域を予約する
	reserved_size *= 2;
	heap = (struct Node**)malloc(sizeof(struct Node*) * reserved_size);
	//古い領域から新しい領域にポインタを全てコピー
	for (size_t i = 0; i < node_num; ++i)
		heap[i] = old_heap[i];
	free(old_heap);
}

struct Node* gc_alloc() {
	if (node_num + 1 > reserved_size)
		reserve();
	struct Node *node = (struct Node*)malloc(sizeof(struct Node));
	heap[node_num++] = node;
	return node;
}
