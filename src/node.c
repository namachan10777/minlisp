#include <node.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint32_t pair_len(struct Node pair) {
	if (pair.tag != Pair) return 0;
	return 1 + pair_len(*pair.pair.cdr);
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
				case Add :
				case Sub :
				case Mul :
				case Div :
				case Mod :
				case Car :
				case Cdr :
				case Cons :
				case List :
					break;
			}
		}
	case SForm: {
			switch (node.sform) {
				case If :
				case Let :
				case Quote :
				case Defun :
				case Lambda :
					break;
			}
		}
	}
	return NULL;
}
