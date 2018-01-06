#include <node.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "util.h"
#include "gc.h"
#include "env.h"

int sexp_len(struct Node pair) {
	if (pair.tag != Pair) return 0;
	return 1 + sexp_len(*pair.pair.cdr);
}

struct Node* idx(struct Node* list, int idx) {
	int i = 0;
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
			if (node.num > 1.0f)
				len = (int)ceilf(log10f(abs(node.num) + 0.1f));
			else
				len = 1;
			// 符号が負の場合は一文字増える
			if (node.num < 0.0f) ++len;
			// '.' +  "xxx" '\0'
			len += 4;
			char *buf = malloc(sizeof(char) * len);
			sprintf(buf, "%.3f", node.num);
			return buf;
		}
	case Bool: {
		if (node.boolean) return deep_copy("true");
		else return deep_copy("false");
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
			int arg_str_length = 0;
			for (int i = 0; i < node.fun.arg_num; ++i) {
				arg_str_length += strlen(node.fun.args[i]) + 1;
			}
			--arg_str_length;
			char* body_str = pp(*node.fun.body);
			int len = 6 + 1 + arg_str_length + 1 + strlen(body_str);
			char* buf = malloc(sizeof(char) * len);
			sprintf(buf, "(lambda ");
			for(int i = 0; i < node.fun.arg_num; ++i) {
				strcat(buf, node.fun.args[i]);
				strcat(buf, " ");	
			}
			strcat(buf, body_str);
			strcat(buf, ")");
			free(body_str);
			return buf;	
		}
	case BFun: {
			switch (node.bfun) {
				case Add : return deep_copy("+");
				case Sub : return deep_copy("-");
				case Mul : return deep_copy("*");
				case Div : return deep_copy("/");
				case Mod : return deep_copy("%");
				case Not : return deep_copy("not");
				case And : return deep_copy("and");
				case Or : return deep_copy("or");
				case Gret: return deep_copy("<");
				case Less: return deep_copy(">");
				case Eq: return deep_copy("=");
				case Car : return deep_copy("car");
				case Cdr : return deep_copy("cdr");
				case Cons : return deep_copy("cons");
				case List : return deep_copy("list");
				case Print : return deep_copy("print");
			}
		}
	case SForm: {
			switch (node.sform) {
				case If : return deep_copy("if");
				case Let : return deep_copy("let");
				case Quote : return deep_copy("quote");
				case Defun : return deep_copy("defun");
				case Lambda : return deep_copy("lambda");
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
struct Node* alloc_fun(char** args, int arg_num, struct Node* body) {
	struct Node* node = gc_alloc();
	node->tag = Fun;
	node->fun.args = args;
	node->fun.arg_num = arg_num;
	node->fun.body = body;
	node->fun.pos = current_fptr();
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
