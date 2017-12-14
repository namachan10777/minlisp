#ifndef __AST_H__
#define __AST_H__

#include <stdint.h>
#include <stdbool.h>

enum Tag {
	Nil,
	Num,
	Str,
	Symbol,
	Sexp
};

struct Node;

struct List {
	//常識的に考えて2^32を超える引数は無いはず
	struct Node **sexps;
	uint32_t sexp_num;
};

struct Node {
	// GC tag
	bool visited;
	enum Tag tag;
	union {
		double num;
		char* str;
		char* symbol;
		//持っているAstの参照の配列。envに登録してある。
		struct List sexp;
	};
};

#endif
