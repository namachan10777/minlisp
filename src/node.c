#include <node.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "gc.h"

uint32_t sexp_len(struct Node pair) {
	if (pair.tag != Pair) return 0;
	return 1 + sexp_len(*pair.pair.cdr);
}

struct Node* idx(struct Node* list, size_t idx) {
	size_t i = 0;
	ITER_REF(node, list) {
		if (idx == i++)
			return node;
	}
	return NULL;
}

char* pp(struct Node node) {
	switch (node.tag) {
	case Nil: {
			// "nil" + '\0'
			int len = 4;
			char* buf = malloc(sizeof(char) * len);
			sprintf(buf, "%s", "nil");
			return buf;
		}
	case Num: {
			int len;
			// 桁数計算
			if (node.num < 1.0f)
				len = ceilf(log10f(node.num));
			else
				len = 0;
			// 符号が負の場合は一文字増える
			if (node.num < 0) ++len;
			// '.' +  "xxx" '\0'
			len += 4;
			char *buf = malloc(sizeof(char) * len);
			sprintf(buf, "%.3f", node.num);
			return buf;
		}
	case Bool: {
		if (node.boolean) return format("true");
		else return format("false");
	}
	case Symbol: {
			// sym + '\0'
			int len = strlen(node.symbol) + 1;
			char *buf = malloc(sizeof(char) * len);
			strcpy(buf, node.symbol);
			return buf;
		}
	case Str: {
			//'"' + str + '"' + '\0'
			int len = strlen(node.str) + 3;
			char *buf = malloc(sizeof(char) * len);
			sprintf(buf, "\"%s\"", node.str);
			return buf;
		}
	case Pair: {
			char* car_str = pp(*node.pair.car);
			char* cdr_str = pp(*node.pair.cdr);
			int len = 1 + strlen(car_str) + 1 + strlen(cdr_str) + 1;
			char *buf = malloc(sizeof(char) * len);
			sprintf(buf, "(%s %s)", car_str, cdr_str);
			free(cdr_str);
			free(car_str);
			return buf;
		}
	case Fun: {
			char* car_str = pp(*node.pair.car);
			char* cdr_str = pp(*node.pair.cdr);
			int len = 1 + 7 + strlen(car_str) + 1 + strlen(cdr_str) + 1;
			char *buf = malloc(sizeof(char) * len);
			sprintf(buf, "(lambda %s %s)", car_str, cdr_str);
			free(cdr_str);
			free(car_str);
			return buf;		
		}
	case BFun: {
			switch (node.bfun) {
				case Add : return format("+");
				case Sub : return format("-");
				case Mul : return format("*");
				case Div : return format("/");
				case Mod : return format("%");
				case Not : return format("not");
				case And : return format("and");
				case Or : return format("or");
				case Car : return format("car");
				case Cdr : return format("cdr");
				case Cons : return format("cons");
				case List : return format("list");
			}
		}
	case SForm: {
			switch (node.sform) {
				case If : return format("if");
				case Let : return format("let");
				case Quote : return format("quote");
				case Defun : return format("defun");
				case Lambda : return format("lambda");
			}
		}
	}
	return NULL;
}

struct Node* alloc_nil() {
	struct Node* node = gc_alloc();
	node->tag = Nil;
	return node;
}
struct Node* alloc_bool(bool boolean) {
	struct Node* node = gc_alloc();
	node->tag = Bool;
	node->boolean = boolean;
	return node;
}
struct Node* alloc_num(float num) {
	struct Node* node = gc_alloc();
	node->tag = Num;
	node->num = num;
	return node;
}
struct Node* alloc_symbol(char* symbol) {
	struct Node* node = gc_alloc();
	node->tag = Symbol;
	node->symbol = symbol;
	return node;
}
struct Node* alloc_str(char* str) {
	struct Node* node = gc_alloc();
	node->tag = Str;
	node->str = str;
	return node;
}
struct Node* alloc_pair(struct Node* car, struct Node* cdr) {
	struct Node* node = gc_alloc();
	node->tag = Pair;
	node->pair.car = car;
	node->pair.cdr = cdr;
	return node;
}
struct Node* alloc_fun(struct Node* args, struct Node* body) {
	struct Node* node = gc_alloc();
	node->tag = Fun;
	node->fun.args = args;
	node->fun.body = body;
	return node;
}
struct Node* alloc_bfun(enum BuiltinFun bfun) {
	struct Node* node = gc_alloc();
	node->tag = BFun;
	node->bfun = bfun;
	return node;
}
struct Node* alloc_sform(enum SpecialForm sform) {
	struct Node* node = gc_alloc();
	node->tag = SForm;
	node->sform = sform;
	return node;
}
