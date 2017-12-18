#include <stdio.h>
#include <gc.h>
#include <node.h>
#include <parser.h>

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("ファイルを一つだけ渡してください\n");
		return -1;
	}
	gc_init();
	struct Node* node = start_parse(argv[1]);
	char* str = pp(node);
	printf("%s\n", str);
	printf("ファイル名 : %s", argv[1]);
	env_init();
}
