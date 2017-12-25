#ifndef __AST_H__
#define __AST_H__

#include <stdint.h>
#include <stdbool.h>

enum Tag {
	Nil,
	Num,
	Str,
	Symbol,
	Pair,
	Fun,
	BFun,
	SForm
};

enum BultinFun {
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	Cons,
	Cdr,
	Car,
	List
};

enum SpecialForm {
	If,
	Let,
	Quote,
	Defun,
	Lambda
};

struct Node;

struct Pair{
	struct Node* car;
	struct Node* cdr;
};

struct Fun {
	struct Node* args;
	struct Node* body;
};

struct Node {
	// GC tag
	bool visited;
	enum Tag tag;
	union {
		double num;
		char* str;
		char* symbol;
		struct Pair pair;
		struct Fun fun;
		enum BultinFun bfun;
		enum SpecialForm sform;
	};
};

char* pp (struct Node node);
uint32_t sexp_len(struct Node sexp);

#endif
