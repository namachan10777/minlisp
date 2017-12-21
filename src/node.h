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

struct Sexp{
	struct Node* car;
	struct Node* cdr;
};

struct Node {
	// GC tag
	bool visited;
	enum Tag tag;
	union {
		double num;
		char* str;
		char* symbol;
		struct Sexp sexp;
	};
};

char* pp (struct Node node);
uint32_t sexp_len(struct Node sexp);

#endif
