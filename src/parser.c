#include <parser.h>
#include <stddef.h>
#include <stdlib.h>
#include "gc.h"
#include "node.h"
#include "util.h"

char* str_slice(char* buf, const char* str, int start, int64_t end) {
	for (int from = start, to = 0; from < end; ++from, ++to) {
		buf[to] = str[from];
	}
	buf[end-start] = '\0';
	return buf;
}

char* read_all(FILE* fp) {
	int buf_size = 256, length = 0;
	char* buf;
	INIT(char, buf, buf_size);
	char c;
	while((c = fgetc(fp)) != EOF) {
		//バッファから溢れる場合にはバッファを拡張する。
		APPEND(char, buf, buf_size, length, c);
	}
	APPEND(char, buf, buf_size, length, EOF);
	return buf;
}

bool is_digit(const char c) {
	return (int)c >= '0' && (int)c <= '9';
}

bool is_white(const char c) {
	return c == ' ' || c == '\n' || c == '\t';
}

struct Node* parse_num(const char *str, int *idx) {
	int start = *idx, end;
	char buf[256];
	for (;is_digit(str[*idx]) || str[*idx] == '.'; ++(*idx)) {
		end = *idx + 1;
	}
	char* slice = str_slice(buf, str, start, end);

	return alloc_num(atof(slice));
}

struct Node* parse_symbol(const char *str, int *idx) {
	int start = *idx, end; 
	for (;!is_white(str[*idx]) && str[*idx] != '(' && str[*idx] != ')' && str[*idx] != EOF; ++(*idx)) {
		end = *idx+1;
	}
	char* buf = malloc(sizeof(struct Node*) * (end - start + 1));
	char* slice = str_slice(buf, str, start, end); 

	if (strcmp(slice, "nil") == 0) {
		free(buf);
		return alloc_nil();
	}
	else if (strcmp(slice, "true") == 0) {
		free(buf);
		return alloc_bool(true);
	}
	else if (strcmp(slice, "false") == 0) {
		free(buf);
		return alloc_bool(false);
	}
	else
		return alloc_symbol(slice);
}

struct Node* parse_str(const char *str, int *idx) {
	int start = *idx, end;
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
	return alloc_str(slice);
}

struct Node** reserve_nodes(struct Node** nodes, int *buf_size) {
	*buf_size = *buf_size * 2;
	struct Node** buf = malloc(sizeof(struct Node*) * *buf_size * 2);
	for (int i = 0; i < *buf_size; ++i)
		buf[i] = nodes[i];
	free(nodes);
	return buf;
}

struct Node* parse(const char *str, int *idx) {
	while(is_white(str[*idx])) ++(*idx);
	if (str[*idx] == EOF) return NULL;
	else if (str[*idx] == '(') {
		//要素を最後の要素のcdrに代入していく
		//最初の要素は使わないので静的に確保していい
		struct Node** nodes;
		int nodes_reserved_size = 32, nodes_size = 0;
		INIT(struct Node*, nodes, nodes_reserved_size);
		bool is_pair = false;
		for (*idx = *idx+1;;) {
			//reduce動作
			if (str[*idx] == ')') {
 				++(*idx);
				break;
			}
			else if (str[*idx] == '.') {
				if (is_pair) return NULL;
				++(*idx);
				is_pair = true;
				continue;
			}
			else if (str[*idx] == EOF) {
				return NULL;
			}
			else if (is_white(str[*idx])) {
 				++(*idx);
				continue;
			}
			else {
				struct Node* elm = parse(str, idx);
				if (elm == NULL) return NULL;
				APPEND(struct Node*, nodes, nodes_reserved_size, nodes_size, elm);
			}
		}
		if (nodes_size == 1) {
			return nodes[0];
		}
		else if (!is_pair) {
			struct Node* node = alloc_nil();
			for (int idx = nodes_size - 1; idx >= 0; --idx) {
				node = alloc_pair(nodes[idx], node);
			}
			return node;
		}
		else if (is_pair && nodes_size == 2) {
			return alloc_pair(nodes[0], nodes[1]);
		}
		else {
			return NULL;
		}
	}
	else if (str[*idx] == '"') return parse_str(str, idx);
	else if (is_digit(str[*idx])) return parse_num(str, idx);
	else return parse_symbol(str, idx);
}

struct Node* parse_src(const char* str, int* idx) {
	struct Node** progs;
	int progs_reserved_size = 32, progs_size = 0;
	INIT(struct Node*, progs, progs_reserved_size);
	while(str[*idx] != EOF) {
		if (is_white(str[*idx])) {
			++(*idx);
		}
		else {
			struct Node* prog = parse(str, idx);
			if (prog == NULL) return NULL;
			APPEND(struct Node*, progs, progs_reserved_size, progs_size, prog);
		}
	}
	struct Node* args = alloc_nil();
	for (int idx = progs_size - 1; idx >= 0; --idx) {
		args = alloc_pair(progs[idx], args);
	}
	return alloc_pair(alloc_bfun(Progn), args);
}

//失敗したらNULLを返す(設計とは)
struct Node* start_parse (const char* file_name) {
	FILE* fp = fopen(file_name, "r");
	if (fp == NULL) return NULL;
	char* buf = read_all(fp);
	int idx = 0;
	struct Node* node = parse_src(buf, &idx);
	free(buf);
	fclose(fp);
	return node;
}
