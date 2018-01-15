#include "eval.h"
#include "node.h"
#include "env.h"
#include "util.h"
#include "gc.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ASSERT(cond, ext) do{ \
	if (!(cond)){ \
		fprintf(stderr, "%s:%d %s\n"); \
		ext; \
		return NULL; \
	}}while(false)

//組み込み関数
struct Node* eval_add(struct Node* args) {
	double sum = 0.0f;
	ITER(num, args) {
		num = eval(num);
		ASSERT(num->tag == Num, fprintf(stderr, "type err. Add function required Num\n"));
		sum += num->num;
	}
	return alloc_num(sum);
}

struct Node* eval_sub(struct Node* args) {
	if (sexp_len(*args) == 0) return alloc_num(0.0f);
	else if (sexp_len(*args) == 1) {
		struct Node* num = eval(args->pair.car);
		ASSERT (num->tag == Num, fprintf(stderr, "type err. Sub function required Num\n"));
		return alloc_num(-num->num);
	}
	else {
		struct Node* num = eval(args->pair.car);
		ASSERT (num->tag == Num, fprintf(stderr, "type err. Sub function required Num\n"));
		double sub = num->num;
		ITER(num, args->pair.cdr) {
			num = eval(num);
			ASSERT(num->tag == Num, fprintf(stderr, "type err. Sub function required Num\n"));
			sub -= num->num;
		}
		return alloc_num(sub);
	}
}

struct Node* eval_mul(struct Node* args) {
	double pro = 1.0f;
	ITER(num, args) {
		num = eval(num);
		ASSERT (num->tag == Num, fprintf(stderr, "type err. Mul function required Num\n"));
		pro *= num->num;
	}
	return alloc_num(pro);
}

struct Node* eval_div(struct Node* args) {
	if (sexp_len(*args) == 0) return alloc_num(1.0f);
	else if (sexp_len(*args) == 1) {
		struct Node* num = eval(args->pair.car);
		ASSERT (num->tag == Num, fprintf(stderr, "type err. Div function required Num\n"));
		return alloc_num(num->num);
	}
	else {
		struct Node* num = eval(args->pair.car);
		ASSERT(num->tag == Num, fprintf(stderr, "type err. Div function required Num\n"));
		double div = num->num;
		ITER(num, args->pair.cdr) {
			num = eval(num);
			ASSERT(num->tag == Num, fprintf(stderr, "type err. Div function required Num\n"));
			ASSERT(num->num != 0.0f && num->num != -0.0f, fprintf(stderr, "Zero divertion exception.\n"));
			div /= num->num;
		}
		return alloc_num(div);
	}
}

struct Node* eval_mod(struct Node* args) {
	if (sexp_len(*args) <= 1) return alloc_num(0.0f);
	else {
		struct Node* num = eval(args->pair.car);
		ASSERT(num->tag == Num, fprintf(stderr, "type err. Mod function required Num\n"));
		double mod = num->num;
		ITER(num, args->pair.cdr) {
			num = eval(num);
			ASSERT(num->tag == Num, fprintf(stderr, "type err. Mod function required Num\n"));
			ASSERT(num->num != 0.0f && num->num != -0.0f, fprintf(stderr, "Zero mod exception\n"));
			mod = fmod(mod, num->num);
		}
		return alloc_num(mod);
	}
}

struct Node* eval_not(struct Node* arg) {
	struct Node* b = eval(arg->pair.car);
	ASSERT(b->tag == Bool, fprintf(stderr, "type err. \"not\" required Bool\n"));
	return alloc_bool(!b->boolean);
}

struct Node* eval_and(struct Node* args) {
	bool acc = true;
	ITER(b, args) {
		b = eval(b);
		ASSERT(b->tag == Bool, fprintf(stderr, "type err. \"and\" required Bool\n"));
		acc &= b->boolean;
	}
	return alloc_bool(acc);
}

struct Node* eval_or(struct Node* args) {
	bool acc = false;
	ITER(b, args) {
		b = eval(b);
		ASSERT (b->tag == Bool, fprintf(stderr, "type err. \"not\" required Bool\n"));
		acc |= b->boolean;
	}
	return alloc_bool(acc);
}

struct Node* eval_gret(struct Node* args) {
	ASSERT (sexp_len(*args) >= 2, fprintf(stderr, "\"<\" required two or more argments.\n"));
	struct Node* left = eval(args->pair.car);
	ASSERT (left->tag == Num, fprintf(stderr, "\"<\" argments required Num\n"));
	ITER(right, args->pair.cdr) {
		right = eval(right);
		ASSERT (right->tag == Num, fprintf(stderr, "\"<\" argments required Num\n"));
		if (left->num >= right->num) {
			return alloc_bool(false);
		}
	}
	return alloc_bool(true);
}

struct Node* eval_less(struct Node* args) {
	ASSERT (sexp_len(*args) >= 2, fprintf(stderr, "\">\" required two or more argments.\n"));
	struct Node* left = eval(args->pair.car);
	ASSERT(left->tag == Num, fprintf(stderr, "\">\" argments required Num\n"));
	ITER(right, args->pair.cdr) {
		right = eval(right);
		ASSERT(right->tag == Num, fprintf(stderr, "\">\" argments required Num\n"));
		if (left->num <= right->num) {
			return alloc_bool(false);
		}
		left = right;
	}
	return alloc_bool(true);
}

bool eq(struct Node* left, struct Node* right) {
	if (right->tag != left->tag) return false;
	switch(left->tag) {
	case Nil: return true;
	case Bool: return left->boolean == right->boolean;
	case Num: return left->num == right->num;
	case Str: return strcmp(left->str, right->str) == 0;
	case Symbol: return false;
	case Pair: {
			return eq(right->pair.car, left->pair.car) && eq(right->pair.cdr, left->pair.cdr);
		}
	case SForm: {
			return left->sform == Quote && right->sform == Quote;
		}
	case BFun:
	case Fun: {
			return false;
		}
	}
	return false;
}

struct Node* eval_eq(struct Node* args) {
	ASSERT(sexp_len(*args) >= 2, fprintf(stderr, "\">\" required two or more argments.\n"));
	struct Node* left = eval(args->pair.car);
	bool acc = true;
	ITER(right, args->pair.cdr) {
		right = eval(right);
		acc &= eq(left, right);
	}
	return alloc_bool(acc);
}

struct Node* eval_car(struct Node* arg) {
	ASSERT (sexp_len(*arg) == 1, fprintf(stderr, "\"car\" required a argments.\n"));
	struct Node* pair = eval(arg->pair.car);
	ASSERT (pair->tag == Pair, fprintf(stderr, "\"car\" argment required Pair\n"));
	return pair->pair.car;
}

struct Node* eval_cdr(struct Node* arg) {
	ASSERT (sexp_len(*arg) == 1, fprintf(stderr, "\"cdr\" required a argments.\n"));
	struct Node* pair = eval(arg->pair.car);
	ASSERT (pair->tag == Pair, fprintf(stderr, "car\" argment required Pair.\n"));
	return pair->pair.cdr;
}

struct Node* eval_cons(struct Node* args) {
	ASSERT(sexp_len(*args) >= 2, fprintf(stderr, "\"cons\" required two argments.\n"));
	struct Node* car = eval(args->pair.car);
	struct Node* cdr = eval(args->pair.cdr->pair.car);
	return alloc_pair(car, cdr);
}

struct Node* eval_list(struct Node* args) {
	struct Node* node = alloc_nil();
	ITER(elm, args) {
		elm = eval(elm);
		node = alloc_pair(elm, node);
	}
	return node;
}


//特殊形式
struct Node* eval_if(struct Node* cond, struct Node* sexp1, struct Node* sexp2) {
	cond = eval(cond);
	ASSERT (cond->tag == Bool, fprintf(stderr, "\"if\"'s condition required Bool.\n"));
	if  (cond->boolean) {
		return eval(sexp1);	
	}
	else {
		return eval(sexp2);
	}
}

struct Node* eval_let(char* symbol, struct Node* def, struct Node* exp) {
	into_scope();
	def = eval(def);
	resist(symbol, def);
	struct Node* result = eval(exp);
	exit_scope(result);
	return result;
}

struct Node* eval_letS(char* symbol, struct Node* def) {
	def = eval(def);
	resist(symbol, def);
	return alloc_nil();
}

//引数の型チェック
struct Node* eval_lambda(struct Node* args, struct Node* body) {
	if (args->tag == Symbol) {
		char** fun_args = malloc(sizeof(char*) * 1);
		fun_args[0] = args->symbol;
		return alloc_fun(fun_args, 1, body);
	}
	else if (args->tag == Nil) {
		return alloc_fun(NULL, 0, body);
	}
	else if (args->tag == Pair) {
		int fun_arg_num = sexp_len(*args);
		char** fun_args = malloc(sizeof(char*) * fun_arg_num);
		int i = 0;
		ITER(arg, args) {
			ASSERT (arg->tag == Symbol, fprintf(stderr, "formal argments is incorrect.\n"));
			fun_args[i++] = deep_copy(arg->symbol);
		}
		return alloc_fun(fun_args, fun_arg_num, body);
	}
	else {
		fprintf(stderr, "formal argments is incorrect.\n");
		return NULL;
	}
}

struct Node* eval_defun(char* symbol, struct Node* args, struct Node* body, struct Node* exp) {
	struct Node* lambda = eval_lambda(args, body);
	return eval_let(symbol, lambda, exp);
}

struct Node* eval_defunS(char* symbol, struct Node* args, struct Node* body) {
	struct Node* lambda = eval_lambda(args, body);
	return eval_letS(symbol, lambda);
}

struct Node* eval_fun(struct Node* fun, struct Node* args) {
	ASSERT (fun->tag == Fun, fprintf(stderr, "It is not a function.\n"));
	ASSERT (fun->fun.arg_num == sexp_len(*args), fprintf(stderr, "formal argments number and real argments number does not correspond.\n"));
	struct Node** real_args = malloc(sizeof(struct Node*) * fun->fun.arg_num);
	int idx = 0;
	ITER(node, args) {
		real_args[idx++] = eval(node);
	}
	into_func(fun->fun.pos);
	for (int i = 0; i < fun->fun.arg_num; ++i) {
		resist(fun->fun.args[i], real_args[i]);
	}
	free(real_args);
	struct Node* result = eval(fun->fun.body);
	exit_func(result);
	return result;
}

struct Node* eval_print(struct Node* arg) {
	ASSERT (sexp_len(*arg) == 1, fprintf(stderr, "\"print\" required a argment.\n"));
	struct Node* evaled = eval(arg->pair.car);
	puts(pp(*evaled));
	return alloc_nil();
}

struct Node* eval_progn(struct Node* args) {
	struct Node* retval;
	ITER(node, args) {
		retval = eval(node);
	}
	return retval;
}

struct Node* eval_bfun(enum BuiltinFun bfun, struct Node* args) {
	switch(bfun) {
	case Add: return eval_add(args);
	case Sub: return eval_sub(args);
	case Mul: return eval_mul(args);
	case Div: return eval_div(args);
	case Mod: return eval_mod(args);
	case Not: return eval_not(args);
	case And: return eval_and(args);
	case Or: return eval_or(args);
	case Gret: return eval_gret(args);
	case Less: return eval_less(args);
	case Eq: return eval_eq(args);
	case Car: return eval_car(args);
	case Cdr: return eval_cdr(args);
	case Cons: return eval_cons(args);
	case List: return eval_list(args);
	case Print: return eval_print(args);
	case Progn: return eval_progn(args);
	}
	return NULL;
}

struct Node* eval_sform(enum SpecialForm sform, struct Node* args) {
	switch(sform) {
	case If: {
			ASSERT (sexp_len(*args) == 3, fprintf(stderr, "\"if\" required three or more argments.\n"));
			return eval_if(idx(args, 0), idx(args, 1), idx(args, 2));
		}
	case Let: {
			ASSERT (sexp_len(*args) == 2 || sexp_len(*args) == 3, fprintf(stderr, "\let\" required two or three argments.\n"));
			ASSERT (args->pair.car->tag == Symbol, fprintf(stderr, "\"let\"'s first argment must be Symbol.\n"));
			if (sexp_len(*args) == 2)
				return eval_letS(idx(args, 0)->symbol, idx(args, 1));
			else 
				return eval_let(idx(args, 0)->symbol, idx(args, 1), idx(args, 2));
		}
	case Defun:{
			ASSERT (args->pair.car->tag == Symbol, fprintf(stderr, "\"defun\"'s first argment must be Symbol.\n"));
			ASSERT (sexp_len(*args) == 3 || sexp_len(*args) == 4, fprintf(stderr, "\"defun\" require three or two argments.\n"));
			if (sexp_len(*args) == 3)
				return eval_defunS(idx(args, 0)->symbol, idx(args, 1), idx(args, 2));
			else
				return eval_defun(idx(args, 0)->symbol, idx(args, 1), idx(args, 2), idx(args, 3));
		}
	case Lambda:{
			ASSERT (sexp_len(*args) == 2, fprintf(stderr, "\"lambda\" require two argments.\n"));
			return eval_lambda(idx(args, 0), idx(args, 1));
		}
	case Quote:{
			return args;
		}
	}
	return NULL;
}

struct Node* eval (struct Node* node) {
	gc_collect();
	switch (node->tag) {
	//nil, 数値, 文字列, 関数, 特殊形式は評価されてもそのまま
	case Nil:
	case Bool:
	case Num:
	case Fun:
	case BFun:
	case SForm:
	case Str: {
			return node;
		}
	case Symbol: {
			return find(node->symbol);
		}
	case Pair: {
			struct Node* fun = eval(node->pair.car);
			//参照なら参照を剥がす
			if (fun->tag == Fun) {
				return eval_fun(fun, node->pair.cdr);
			}
			else if (fun->tag == BFun) {
				return eval_bfun(fun->bfun, node->pair.cdr);
			}
			else if (fun->tag == SForm) {
				return eval_sform(fun->sform, node->pair.cdr);
			}
			else {
				ASSERT(false, fprintf(stderr, "list cannot evaluate because it is not function apprication\n"));
			}
		}
	}
	return NULL;
}
