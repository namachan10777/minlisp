#include <parser.h>
#include <stddef.h>
#include <stdlib.h>
#include "gc.h"
#include "node.h"

char* reserve_str(char* str, size_t *buf_size) {
	char* buf = malloc(sizeof(char) * *buf_size * 2);
	for (size_t i = 0; i < *buf_size; ++i)
		buf[i] = str[i];
	free(str);
	return buf;
}

char* str_slice(char* buf, const char* str, size_t start, size_t end) {
	for (size_t from = start, to = 0; from < end; ++from, ++to) {
		buf[to] = str[from];
	}
	buf[end] = '\0';
	return buf;
}

char* read_all(FILE* fp) {
	size_t buf_size = 256, length = 0;
	char* buf = malloc(sizeof(char) * buf_size);
	char c;
	while((c = fgetc(fp)) != EOF) {
		//バッファから溢れる場合にはバッファを拡張する。
		if (length >= buf_size)
			buf = reserve_str(buf, &buf_size);	
		buf[length++] = c;
	}
	if (length+1 >= buf_size)
		buf = reserve_str(buf, &buf_size);
	buf[length] = EOF;
	return buf;
}

bool is_digit(const char c) {
	return (int)c >= '0' && (int)c <= '9';
}

bool is_white(const char c) {
	return c == ' ' || c == '\n' || c == '\t';
}

struct Node* parse_num(const char *str, size_t *idx) {
	size_t start = *idx, end;
	char buf[256];
	for (;is_digit(str[*idx]) || str[*idx] == '.'; ++(*idx)) {
		end = *idx + 1;
	}
	char* slice = str_slice(buf, str, start, end);
	float f = atof(slice);
	struct Node* node = gc_alloc();

	node->tag = Num;
	node->num = f;

	return node;
}

struct Node* parse_symbol(const char *str, size_t *idx) {
	size_t start = *idx, end; 
	for (;!is_white(str[*idx]) && str[*idx] != '(' && str[*idx] != ')' && str[*idx] != EOF; ++(*idx)) {
		end = *idx+1;
	}
	char* buf = malloc(sizeof(struct Node*) * (end - start + 1));
	char* slice = str_slice(buf, str, start, end); 
	struct Node* node = gc_alloc();

	if (strcmp(slice, "nil")) {
		node->tag = Symbol;
		node->symbol = slice;
	}
	else {
		node->tag = Nil;
	}
	return node;
}

struct Node* parse_str(const char *str, size_t *idx) {
	size_t start = *idx, end;
	for (++(*idx);; ++(*idx)) {
		if (str[*idx] == EOF) return NULL;
		if (str[*idx] == '\\' && str[*idx+1] == '"') *idx += 2;
		if (str[*idx] == '"') {
			end = ++(*idx);
			break;
		}
	}
	char* buf = malloc(sizeof(char) * (end - start - 1));
	char* slice = str_slice(buf, str, start+1, end-1);
	struct Node* node = gc_alloc();

	node->tag = Str;
	node->str = slice;

	return node;
}

struct Node** reserve_nodes(struct Node** nodes, size_t *buf_size) {
	*buf_size = *buf_size * 2;
	struct Node** buf = malloc(sizeof(struct Node*) * *buf_size * 2);
	for (size_t i = 0; i < *buf_size; ++i)
		buf[i] = nodes[i];
	free(nodes);
	return buf;
}

struct Node* parse(const char *str, size_t *idx) {
	if (str[*idx] == EOF) return NULL;
	else if (str[*idx] == '(') {
		struct Node* node = gc_alloc();
		node->tag = Nil;
		for (*idx = *idx+1;;) {
			//reduce動作
			if (str[*idx] == ')') {
 				++(*idx);
				break;
			}
			else if (str[*idx] == EOF) {
				return NULL;
			}
			else if (is_white(str[*idx])) {
 				++(*idx);
				continue;
			}
			else {
				struct Node* car = parse(str, idx);
				if (car == NULL) return NULL;
				struct Node* sexp = gc_alloc();
				sexp->sexp.car = car;
				sexp->sexp.cdr = node;
				sexp->tag = Sexp;
				node = sexp;
			}
		}
		return node;
	}
	else if (str[*idx] == '"') return parse_str(str, idx);
	else if (is_digit(str[*idx])) return parse_num(str, idx);
	else return parse_symbol(str, idx);
}

//失敗したらNULLを返す(設計とは)
struct Node* start_parse (const char* file_name) {
	FILE* fp = fopen(file_name, "r");
	if (fp == NULL) return NULL;
	char* buf = read_all(fp);
	size_t idx = 0;
	struct Node* node = parse(buf, &idx);
	free(buf);
	fclose(fp);
	return node;
}
