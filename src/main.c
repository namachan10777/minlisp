#include <stdio.h>
#include <gc.h>
#include <node.h>
#include <parser.h>
#include <env.h>
#include "eval.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("ファイルを一つだけ渡してください\n");
		return -1;
	}
	gc_init();
	env_init();
	struct Node* node = start_parse(argv[1]);
	if (node == NULL) return -1;
	struct Node* result = eval(node);
	if (result == NULL) return -1;
	gc_collect();
	gc_quit();
	env_quit();
}
