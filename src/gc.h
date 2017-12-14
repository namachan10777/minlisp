#ifndef __GC_H__
#define __GC_H__

#include "node.h"

//初期化(必ず呼ぶこと)
void gc_init();
//回収(未実装)
void gc_collect();
//メモリ確保(mallocではなくこちらから確保しないとGCの管理に入らない)
struct Node* gc_alloc();

#endif
