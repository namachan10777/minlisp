#include "eval.h"
#include "node.h"
#include "env.h"
#include "util.h"
#include "gc.h"
#include "macro.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define ASSERT(cond, ext) do{ \
	if (!(cond)){ \
		fprintf(stderr, "%s:%d %s\n", __FILE__, __LINE__, __func__); \
		ext; \
		return NULL; \
	}}while(false)

#define NULLCHECK(node) do {\
	if (node == NULL) {\
		return NULL; \
	}}while(false)

struct Node* deref(char* key) {
	struct Node *node = find(key);
	ASSERT(node != NULL, fprintf (stderr, "%s : 未定義の変数です\n", key));
	return node;
}

//組み込み関数
struct Node* eval_add(struct Node* args) {
	double sum = 0.0f;
	ITER_REF(num, args) {
		num = eval(num);
		NULLCHECK(num);
		ASSERT(num->tag == Num, fprintf(stderr, "加算関数に数値以外を適用しようとしました\n"));
		sum += num->num;
	}
	return alloc_num(sum);
}

struct Node* eval_sub(struct Node* args) {
	if (sexp_len(*args) == 0) return alloc_num(0.0f);
	else if (sexp_len(*args) == 1) {
		struct Node* num = eval(args->pair.car);
		NULLCHECK(num);
		ASSERT (num->tag == Num, fprintf(stderr, "減算関数に数値以外を適用しようとしました\n"));
		return alloc_num(-num->num);
	}
	else {
		struct Node* num = eval(args->pair.car);
		NULLCHECK(num);
		ASSERT (num->tag == Num, fprintf(stderr, "減算関数に数値以外を適用しようとしました\n"));
		double sub = num->num;
		ITER_REF(num, args->pair.cdr) {
			num = eval(num);
			NULLCHECK(num);
			ASSERT(num->tag == Num, fprintf(stderr, "減算関数に数値以外を適用しようとしました\n"));
			sub -= num->num;
		}
		return alloc_num(sub);
	}
}

struct Node* eval_mul(struct Node* args) {
	double pro = 1.0f;
	ITER_REF(num, args) {
		num = eval(num);
		NULLCHECK(num);
		ASSERT (num->tag == Num, fprintf(stderr, "乗算関数に数値以外を適用しようとしました\n"));
		pro *= num->num;
	}
	return alloc_num(pro);
}

struct Node* eval_div(struct Node* args) {
	if (sexp_len(*args) == 0) return alloc_num(1.0f);
	else if (sexp_len(*args) == 1) {
		struct Node* num = eval(args->pair.car);
		NULLCHECK(num);
		ASSERT (num->tag == Num, fprintf(stderr, "除算関数に数値以外を適用しようとしました\n"));
		return alloc_num(num->num);
	}
	else {
		struct Node* num = eval(args->pair.car);
		NULLCHECK(num);
		ASSERT(num->tag == Num, fprintf(stderr, "除算関数に数値以外を適用しようとしました\n"));
		double div = num->num;
		ITER_REF(num, args->pair.cdr) {
			num = eval(num);
			NULLCHECK(num);
			ASSERT(num->tag == Num, fprintf(stderr, "除算関数に数値以外を適用しようとしました\n"));
			ASSERT(num->num != 0.0f && num->num != -0.0f, fprintf(stderr, "ゼロで除算をしようとしました\n"));
			div /= num->num;
		}
		return alloc_num(div);
	}
}

struct Node* eval_mod(struct Node* args) {
	if (sexp_len(*args) <= 1) return alloc_num(0.0f);
	else {
		struct Node* num = eval(args->pair.car);
		NULLCHECK(num);
		ASSERT(num->tag == Num, fprintf(stderr, "剰余関数に数値以外を適用しようとしました\n"));
		double mod = num->num;
		ITER_REF(num, args->pair.cdr) {
			num = eval(num);
			ASSERT(num->tag == Num, fprintf(stderr, "剰余関数に数値以外を適用しようとしました\n"));
			ASSERT(num->num != 0.0f && num->num != -0.0f, fprintf(stderr, "ゼロでの剰余を求めようとしました\n"));
			mod = fmod(mod, num->num);
		}
		return alloc_num(mod);
	}
}

struct Node* eval_not(struct Node* arg) {
	struct Node* b = eval(arg->pair.car);
	NULLCHECK(b);
	ASSERT(b->tag == Bool, fprintf(stderr, "真偽値型以外にnotは適用できません\n"));
	return alloc_bool(!b->boolean);
}

struct Node* eval_and(struct Node* args) {
	bool acc = true;
	ITER_REF(b, args) {
		b = eval(b);
		NULLCHECK(b);
		ASSERT(b->tag == Bool, fprintf(stderr, "真偽値型以外にandは適用できません\n"));
		acc &= b->boolean;
	}
	return alloc_bool(acc);
}

struct Node* eval_or(struct Node* args) {
	bool acc = false;
	ITER_REF(b, args) {
		b = eval(b);
		NULLCHECK(b);
		ASSERT (b->tag == Bool, fprintf(stderr, "真偽値型以外にorは適用できません\n"));
		acc |= b->boolean;
	}
	return alloc_bool(acc);
}

struct Node* eval_gret(struct Node* args) {
	ASSERT (sexp_len(*args) >= 2, fprintf(stderr, "<には引数が2つ以上必要です\n"));
	struct Node* left = eval(args->pair.car);
	NULLCHECK(left);
	ASSERT (left->tag == Num, fprintf(stderr, "大小関係を比較できるのは数値だけです\n"));
	ITER_REF(right, args->pair.cdr) {
		right = eval(right);
		NULLCHECK(right);
		ASSERT (right->tag == Num, fprintf(stderr, "大小関係を比較できるのは数値だけです\n"));
		if (left->num >= right->num) {
			return alloc_bool(false);
		}
	}
	return alloc_bool(true);
}

struct Node* eval_less(struct Node* args) {
	ASSERT (sexp_len(*args) >= 2, fprintf(stderr, ">には引数が2つ以上必要です\n"));
	struct Node* left = eval(args->pair.car);
	ASSERT(left->tag == Num, fprintf(stderr, "大小関係を比較できるのは数値だけです\n"));
	ITER_REF(right, args->pair.cdr) {
		right = eval(right);
		NULLCHECK(right);
		ASSERT(right->tag == Num, fprintf(stderr, "大小関係を比較できるのは数値だけです\n"));
		if (left->num <= right->num) {
			return alloc_bool(false);
		}
		left = right;
	}
	return alloc_bool(true);
}

bool eq(struct Node* left, struct Node* right) {
	NULLCHECK(left);
	NULLCHECK(right);
	if (right->tag != left->tag) return false;
	switch(left->tag) {
	case Nil: return true;
	case Bool: return left->boolean == right->boolean;
	case Num: return left->num == right->num;
	case Str: return strcmp(left->str, right->str);
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
	NULLCHECK(args);
	ASSERT(sexp_len(*args) >= 2, fprintf(stderr, "=には引数が2つ以上必要です\n"));
	struct Node* left = eval(args->pair.car);
	NULLCHECK(left);
	bool acc = true;
	ITER_REF(right, args->pair.cdr) {
		right = eval(right);
		NULLCHECK(right);
		acc &= eq(left, right);
	}
	return alloc_bool(acc);
}

struct Node* eval_car(struct Node* arg) {
	NULLCHECK(arg);
	ASSERT (arg->tag == Pair, fprintf(stderr, "ドット対以外にはcarを適用できません\n"));
	return arg->pair.car;
}

struct Node* eval_cdr(struct Node* arg) {
	NULLCHECK(arg);
	ASSERT (arg->tag == Pair, fprintf(stderr, "ドット対以外にはcdrを適用できません\n"));
	return arg->pair.car;
}

struct Node* eval_cons(struct Node* args) {
	NULLCHECK(args);
	ASSERT(sexp_len(*args) >= 2, fprintf(stderr, "引数の数が不正です\n"));
	struct Node* car = eval(args->pair.car);
	NULLCHECK(car);
	struct Node* cdr = eval(args->pair.cdr->pair.car);
	NULLCHECK(cdr);
	return alloc_pair(car, cdr);
}

struct Node* eval_list(struct Node* args) {
	NULLCHECK(args);
	struct Node* node = alloc_nil();
	ITER_REF(elm, args) {
		elm = eval(elm);
		NULLCHECK(elm);
		node = alloc_pair(elm, node);
	}
	return node;
}


//特殊形式
struct Node* eval_if(struct Node* cond, struct Node* sexp1, struct Node* sexp2) {
	NULLCHECK(cond);
	NULLCHECK(sexp1);
	NULLCHECK(sexp2);
	cond = eval(cond);
	NULLCHECK(cond);
	ASSERT (cond->tag == Bool, fprintf(stderr, "ifの条件式は真偽値型である必要があります\n"));
	if  (cond->boolean) {
		return eval(sexp1);	
	}
	else {
		return eval(sexp2);
	}
}

struct Node* eval_let(char* symbol, struct Node* def, struct Node* exp) {
	NULLCHECK(symbol);
	NULLCHECK(def);
	NULLCHECK(exp);
	into_scope();
	def = eval(def);
	NULLCHECK(def);
	resist(symbol, def);
	struct Node* result = eval(exp);
	NULLCHECK(result);
	exit_scope();
	return result;
}

struct Node* eval_letS(char* symbol, struct Node* def) {
	NULLCHECK(symbol);
	NULLCHECK(def);
	NULLCHECK(exp);
	def = eval(def);
	NULLCHECK(def);
	resist(symbol, def);
	return alloc_nil();
}

//引数の型チェック
struct Node* eval_lambda(struct Node* args, struct Node* body) {
	NULLCHECK(args);
	NULLCHECK(body);
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
		ITER_REF(arg, args) {
			ASSERT (arg->tag == Symbol, fprintf(stderr, "仮引数が不正です\n"));
			fun_args[i++] = deep_copy(arg->symbol);
		}
		return alloc_fun(fun_args, fun_arg_num, body);
	}
	else {
		fprintf(stderr, "仮引数が不正です\n");
		return NULL;
	}
}

struct Node* eval_defun(char* symbol, struct Node* args, struct Node* body, struct Node* exp) {
	NULLCHECK(symbol);
	NULLCHECK(args);
	NULLCHECK(body);
	NULLCHECK(exp);
	struct Node* lambda = eval_lambda(args, body);
	return eval_let(symbol, lambda, exp);
}

struct Node* eval_defunS(char* symbol, struct Node* args, struct Node* body) {
	NULLCHECK(symbol);
	NULLCHECK(args);
	NULLCHECK(body);
	NULLCHECK(exp);
	struct Node* lambda = eval_lambda(args, body);
	return eval_letS(symbol, lambda);
}

struct Node* eval_fun(struct Node* fun, struct Node* args) {
	NULLCHECK(fun);
	NULLCHECK(args);
	ASSERT (fun->tag == Fun, fprintf(stderr, "関数ではないです\n"));
	ASSERT (fun->fun.arg_num == sexp_len(*args), fprintf(stderr, "関数の引数の数と渡された引数の数が一致しません\n"));
	start_resist_real_arg();
	for (int i = 0; i < fun->fun.arg_num; ++i) {
		NULLCHECK(fun->fun.args[i]);
		resist_real_arg(fun->fun.args[i], eval(idx(args, i)));
	}
	enter_func(fun->fun.pos);
	struct Node* result = eval(fun->fun.body);
	exit_func();
	return result;
}

struct Node* eval_print(struct Node* arg) {
	ASSERT (sexp_len(*arg) == 1, fprintf(stderr, "printの引数は一つだけです\n"));
	struct Node* evaled = eval(arg->pair.car);
	NULLCHECK(evaled);
	puts(pp(*evaled));
	return alloc_nil();
}

struct Node* eval_progn(struct Node* args) {
	struct Node* retval;
	ITER_REF(node, args) {
		retval = eval(node);
	}
	return retval;
}

struct Node* eval_bfun(enum BuiltinFun bfun, struct Node* args) {
	NULLCHECK(args);
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
	NULLCHECK(args);
	switch(sform) {
	case If: {
			ASSERT (sexp_len(*args) == 3, fprintf(stderr, "ifには3つの引数が必要です\n"));
			return eval_if(idx(args, 0), idx(args, 1), idx(args, 2));
		}
	case Let: {
			ASSERT (sexp_len(*args) == 2 || sexp_len(*args) == 3, fprintf(stderr, "letには2つか3つの引数が必要です\n"));
			ASSERT (args->pair.car->tag == Symbol, fprintf(stderr, "letの第一引数はシンボルでなければいけません\n"));
			if (sexp_len(*args) == 2)
				return eval_letS(idx(args, 0)->symbol, idx(args, 1));
			else 
				return eval_let(idx(args, 0)->symbol, idx(args, 1), idx(args, 2));
		}
	case Defun:{
			ASSERT (args->pair.car->tag == Symbol, fprintf(stderr, "defunの第一引数はシンボルでなければいけません\n"));
			ASSERT (sexp_len(*args) == 3 || sexp_len(*args) == 4, fprintf(stderr, "defunには3つか3つか3つか4つの引数が必要です\n"));
			if (sexp_len(*args) == 3)
				return eval_defunS(idx(args, 0)->symbol, idx(args, 1), idx(args, 2));
			else
				return eval_defun(idx(args, 0)->symbol, idx(args, 1), idx(args, 2), idx(args, 3));
		}
	case Lambda:{
			ASSERT (sexp_len(*args) == 2, fprintf(stderr, "lambdaには2つの引数が必要です\n"));
			return eval_lambda(idx(args, 0), idx(args, 1));
		}
	case Quote:{
			return args;
		}
	}
	return NULL;
}

struct Node* eval (struct Node* node) {
	NULLCHECK(node);
	//gc_collect();
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
			return deref(node->symbol);
		}
	case Pair: {
			struct Node* fun = eval(node->pair.car);
			NULLCHECK(fun);
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
				ASSERT(false, fprintf(stderr, "関数適用以外のリストを評価できません\n"));
			}
		}
	}
	return NULL;
}
