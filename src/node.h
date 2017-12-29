#ifndef __AST_H__
#define __AST_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

enum Tag {
	Bool,
	Nil,
	Num,
	Str,
	Symbol,
	Pair,
	Fun,
	BFun,
	SForm
};

enum BuiltinFun {
	Add,
	Sub,
	Mul,
	Div,
	Mod,
	Not,
	And,
	Or,
	Gret,
	Less,
	Eq,
	Cons,
	Cdr,
	Car,
	List,
	Print
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
	char** args;
	size_t arg_num;
	struct Node* body;
	uint32_t pos;
};

struct Node {
	// GC tag
	bool visited;
	enum Tag tag;
	union {
		bool boolean;
		double num;
		char* str;
		char* symbol;
		struct Pair pair;
		struct Fun fun;
		enum BuiltinFun bfun;
		enum SpecialForm sform;
	};
};

char* pp (struct Node node);
uint32_t sexp_len(struct Node sexp);
struct Node* idx(struct Node* nodes, size_t idx);

struct Node* alloc_nil();
struct Node* alloc_bool(bool boolean);
struct Node* alloc_num(float num);
struct Node* alloc_symbol(char* symbol);
struct Node* alloc_str(char* str);
struct Node* alloc_pair(struct Node* car, struct Node* cdr);
struct Node* alloc_fun(char** args, size_t args_num, struct Node* body);
struct Node* alloc_bfun(enum BuiltinFun);
struct Node* alloc_sform(enum SpecialForm);

#define ITER_REF(x, list) for (struct Node *__sexp__ = list, *x = list->pair.car; __sexp__->tag == Pair; __sexp__ = __sexp__->pair.cdr, x = __sexp__->pair.car)

#endif
