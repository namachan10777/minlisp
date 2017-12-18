#include <node.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char* pp(struct Node* node) {
	char* buf;
	int len = 0;
	//Sexp用
	char**  strs;
	switch (node->tag) {
	case Nil:
		// "nil" + '\0'
		len = 4;
		buf = malloc(sizeof(char) * len);
		buf = "nil";
		return buf;
	case Num:
		// 桁数計算
		if (node->num < 1.0f)
			len = ceilf(log10f(node->num));
		else
			len = 0;
		// 符号が負の場合は一文字増える
		if (node->num < 0) ++len;
		// '.' +  "xxx" '\0'
		len += 4;
		buf = malloc(sizeof(char) * len);
		sprintf(buf, "%.3f", node->num);
		return buf;
	case Symbol:
		// sym + '\0'
		len = strlen(node->symbol) + 1;
		buf = malloc(sizeof(char) * len);
		strcpy(buf, node->symbol);
		return buf;
	case Str:
		//'"' + str + '"' + '\0'
		len = strlen(node->str) + 3;
		buf = malloc(sizeof(char) * len);
		sprintf(buf, "\"%s\"", node->str);
		return buf;
	case Sexp:
		strs = malloc(sizeof(char) * node->sexp.len);
		len = 0;
		for (size_t i = 0; i < node->sexp.len; ++i) {
			strs[i] = pp(node->sexp.sexps[i]);
			len += strlen(strs[i]);
		}
		//子要素 + 空白 + () + '\0'
		len = len + (node->sexp.len - 1) + 3;
		buf = malloc(sizeof(char) * len);
		strcat(buf, "(");
		for (size_t i = 0; i < node->sexp.len; ++i) {
			strcat(buf, strs[i]);
			free(strs[i]);
			if (i < node->sexp.len - 1)
				strcat(buf, " ");
		}
		strcat(buf, ")");
		free(strs);
		return buf;
	}
	return NULL;
}
